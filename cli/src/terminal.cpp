/* linenoise.c -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * You can find the latest source code at:
 *
 *   http://github.com/antirez/linenoise
 *
 * Does a number of crazy assumptions that happen to be true in 99.9999% of
 * the 2010 UNIX computers around.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010-2016, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2013, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ------------------------------------------------------------------------
 *
 * References:
 * - http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 * - http://www.3waylabs.com/nw/WWW/products/wizcon/vt220.html
 *
 * Todo list:
 * - Filter bogus Ctrl+<char> combinations.
 * - Win32 support
 *
 * Bloat:
 * - History search like Ctrl+r in readline?
 *
 * List of escape sequences used by this program, we do everything just
 * with three sequences. In order to be so cheap we may have some
 * flickering effect with some slow terminal, but the lesser sequences
 * the more compatible.
 *
 * EL (Erase Line)
 *    Sequence: ESC [ n K
 *    Effect: if n is 0 or missing, clear from cursor to end of line
 *    Effect: if n is 1, clear from beginning of line to cursor
 *    Effect: if n is 2, clear entire line
 *
 * CUF (CUrsor Forward)
 *    Sequence: ESC [ n C
 *    Effect: moves cursor forward n chars
 *
 * CUB (CUrsor Backward)
 *    Sequence: ESC [ n D
 *    Effect: moves cursor backward n chars
 *
 * The following is used to get the terminal width if getting
 * the width with the TIOCGWINSZ ioctl fails
 *
 * DSR (Device Status Report)
 *    Sequence: ESC [ 6 n
 *    Effect: reports the current cusor position as ESC [ n ; m R
 *            where n is the row and m is the column
 *
 * When multi line mode is enabled, we also use an additional escape
 * sequence. However multi line editing is disabled by default.
 *
 * CUU (Cursor Up)
 *    Sequence: ESC [ n A
 *    Effect: moves cursor up of n chars.
 *
 * CUD (Cursor Down)
 *    Sequence: ESC [ n B
 *    Effect: moves cursor down of n chars.
 *
 * When linenoiseClearScreen() is called, two additional escape sequences
 * are used in order to clear the screen and position the cursor at home
 * position.
 *
 * CUP (Cursor position)
 *    Sequence: ESC [ H
 *    Effect: moves the cursor to upper left corner
 *
 * ED (Erase display)
 *    Sequence: ESC [ 2 J
 *    Effect: clear the whole screen
 *
 */
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cctype>
#include <deque>

#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <plorth/unicode.hpp>
#include <plorth/cli/terminal.hpp>

#define LINENOISE_DEFAULT_HISTORY_MAX_LEN 100
#define LINENOISE_MAX_LINE 4096

static const char* unsupported_term[] = {"dumb","cons25","emacs",NULL};

static struct termios orig_termios; /* In order to restore at exit.*/
static bool rawmode = false; /* For atexit() function to check if restore is needed*/
static bool atexit_registered = false; /* Register atexit just 1 time. */
static std::deque<std::u32string> history_container;
static std::size_t history_max_len = LINENOISE_DEFAULT_HISTORY_MAX_LEN;

namespace
{
  /* The linenoiseState structure represents the state during line editing.
   * We pass this state to functions implementing specific editing
   * functionalities. */
  struct linenoiseState
  {
    int ifd;            /* Terminal stdin file descriptor. */
    int ofd;            /* Terminal stdout file descriptor. */
    /** Edited line buffer. */
    char32_t* buf;
    /** Edited line buffer size. */
    std::size_t buflen;
    std::string prompt; /* Prompt to display. */
    size_t pos;         /* Current cursor position. */
    size_t oldpos;      /* Previous refresh cursor position. */
    size_t len;         /* Current edited line length. */
    size_t cols;        /* Number of columns in terminal. */
    size_t maxrows;     /* Maximum num of rows used so far (multiline mode) */
    int history_index;  /* The history index we are currently editing. */
  };
}

static const int CTRL_A = 1;
static const int CTRL_B = 2;
static const int CTRL_C = 3;
static const int CTRL_D = 4;
static const int CTRL_E = 5;
static const int CTRL_F = 6;
static const int CTRL_K = 11;
static const int CTRL_L = 12;
static const int ENTER = 13;
static const int CTRL_N = 14;
static const int CTRL_P = 16;
static const int CTRL_T = 20;
static const int CTRL_U = 21;
static const int CTRL_W = 23;
static const int ESC = 27;
static const int BACKSPACE = 127;

static void linenoiseAtExit();
static void refreshLine(linenoiseState&);

/* ======================= Low level terminal handling ====================== */

/**
 * Return true if the terminal name is in the list of terminals we know are
 * not able to understand basic escape sequences.
 */
static bool isUnsupportedTerm()
{
  auto term = getenv("TERM");

  if (!term)
  {
    return false;
  }
  for (int i = 0; unsupported_term[i]; ++i)
  {
    if (!strcasecmp(term, unsupported_term[i]))
    {
      return true;
    }
  }

  return false;
}

/**
 * Raw mode: 1960 magic shit.
 */
static int enableRawMode(int fd)
{
  termios raw;

  if (!isatty(STDIN_FILENO))
  {
    goto fatal;
  }
  if (!atexit_registered)
  {
    std::atexit(linenoiseAtExit);
    atexit_registered = true;
  }
  if (tcgetattr(fd, &orig_termios) == -1)
  {
    goto fatal;
  }

  raw = orig_termios;  /* modify the original mode */
  /* input modes: no break, no CR to NL, no parity check, no strip char,
   * no start/stop output control. */
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  /* output modes - disable post processing */
  raw.c_oflag &= ~(OPOST);
  /* control modes - set 8 bit chars */
  raw.c_cflag |= (CS8);
  /* local modes - choing off, canonical off, no extended functions,
   * no signal chars (^Z,^C) */
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  /* control chars - set return condition: min number of bytes and timer.
   * We want read to return every single byte, without timeout. */
  raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

  /* put terminal in raw mode after flushing */
  if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
  {
    goto fatal;
  }

  rawmode = true;

  return 0;

fatal:
  errno = ENOTTY;

  return -1;
}

static void disableRawMode(int fd)
{
  /* Don't even check the return value as it's too late. */
  if (rawmode && tcsetattr(fd, TCSAFLUSH, &orig_termios) != -1)
  {
    rawmode = false;
  }
}

/**
 * Use the ESC [6n escape sequence to query the horizontal cursor position
 * and return it. On error -1 is returned, on success the position of the
 * cursor.
 */
static int getCursorPosition(int ifd, int ofd)
{
  char buf[32];
  int cols, rows;
  unsigned int i = 0;

  /* Report cursor location */
  if (write(ofd, "\x1b[6n", 4) != 4) return -1;

  /* Read the response: ESC [ rows ; cols R */
  while (i < sizeof(buf)-1) {
      if (read(ifd,buf+i,1) != 1) break;
      if (buf[i] == 'R') break;
      i++;
  }
  buf[i] = '\0';

  /* Parse it. */
  if (buf[0] != ESC || buf[1] != '[') return -1;
  if (sscanf(buf+2,"%d;%d",&rows,&cols) != 2) return -1;
  return cols;
}

/**
 * Try to get the number of columns in the current terminal, or assume 80
 * if it fails.
 */
static int getColumns(int ifd, int ofd)
{
  winsize ws;

  if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
  {
    /* ioctl() failed. Try to query the terminal itself. */
    int start;
    int cols;

    /* Get the initial position so we can restore it later. */
    if ((start = getCursorPosition(ifd,ofd)))
    {
      goto failed;
    }

    /* Go to right margin and get position. */
    if (write(ofd,"\x1b[999C",6) != 6)
    {
      goto failed;
    }

    if ((cols = getCursorPosition(ifd,ofd)) == -1)
    {
      goto failed;
    }

    /* Restore position. */
    if (cols > start)
    {
      char seq[32];

      std::snprintf(seq, 32, "\x1b[%dD", cols - start);
      write(ofd, seq, std::strlen(seq));
    }

    return cols;
  } else {
    return ws.ws_col;
  }

failed:
  return 80;
}

/**
 * Clear the screen. Used to handle ctrl+l
 */
static void linenoiseClearScreen()
{
  write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
}

static void refreshLine(linenoiseState& l)
{
  char seq[64];
  const auto plen = l.prompt.length();
  auto buf = l.buf;
  auto len = l.len;
  auto pos = l.pos;
  std::string buffer;

  while ((plen + pos) >= l.cols)
  {
    ++buf;
    --len;
    --pos;
  }

  while (plen + len > l.cols)
  {
    --len;
  }

  // Cursor to left edge.
  buffer.append(1, '\r');

  // Write the prompt and the current buffer content.
  buffer.append(l.prompt);
  buffer.append(plorth::utf8_encode(buf, len));

  // Erase to right
  buffer.append("\x1b[0K");

  // Move cursor to original position.
  std::snprintf(seq, 64, "\r\x1b[%dC", static_cast<int>(pos + plen));
  buffer.append(seq, std::strlen(seq));

  // Can't recover from write error.
  ::write(l.ofd, buffer.c_str(), buffer.length());
}

/**
 * Insert the character 'c' at cursor current position.
 *
 * On error writing to the terminal -1 is returned, otherwise 0.
 */
static bool linenoiseEditInsert(linenoiseState& l, char32_t c)
{
  if (l.len < l.buflen)
  {
    if (l.len == l.pos)
    {
      l.buf[l.pos] = c;
      ++l.pos;
      ++l.len;
      l.buf[l.len] = '\0';
      if (l.prompt.length() + l.len < l.cols)
      {
        const auto encoded = plorth::utf8_encode(std::u32string(&c, 1));

        // Avoid a full update of the line in the trivial case.
        if (write(l.ofd, encoded.c_str(), encoded.length()) == -1)
        {
          return false;
        }
      } else {
        refreshLine(l);
      }
    } else {
      std::memmove(
        static_cast<void*>(l.buf + l.pos + 1),
        static_cast<const void*>(l.buf + l.pos),
        sizeof(char32_t) * (l.len - l.pos)
      );
      l.buf[l.pos] = c;
      ++l.len;
      ++l.pos;
      l.buf[l.len] = '\0';
      refreshLine(l);
    }
  }

  return true;
}

static bool linenoiseEditInsertUTF8(linenoiseState& l, unsigned char initial)
{
  const auto length = plorth::utf8_sequence_length(initial);
  char32_t result;

  if (length == 0)
  {
    return true;
  }
  else if (length > 1)
  {
    unsigned char buffer[length - 1];
    const auto nread = read(l.ifd, buffer, length - 1);

    if (nread < static_cast<long>(length - 1))
    {
      return true;
    }
    switch (length)
    {
      case 2:
        result = static_cast<std::uint32_t>(initial & 0x1f);
        break;

      case 3:
        result = static_cast<std::uint32_t>(initial & 0x0f);
        break;

      case 4:
        result = static_cast<std::uint32_t>(initial & 0x07);
        break;

      case 5:
        result = static_cast<std::uint32_t>(initial & 0x03);
        break;

      case 6:
        result = static_cast<std::uint32_t>(initial & 0x01);
        break;

      default:
        return true;
    }
    for (std::size_t i = 0; i < length - 1; ++i)
    {
      if ((buffer[i] & 0xc0) != 0x80)
      {
        return true;
      }
      result = (result << 6) | (buffer[i] & 0x3f);
    }
  } else {
    result = static_cast<std::uint32_t>(initial);
  }

  return linenoiseEditInsert(l, result);
}

/* Move cursor on the left. */
static void linenoiseEditMoveLeft(linenoiseState& l)
{
  if (l.pos > 0)
  {
    --l.pos;
    refreshLine(l);
  }
}

/* Move cursor on the right. */
static void linenoiseEditMoveRight(linenoiseState& l)
{
  if (l.pos != l.len)
  {
    ++l.pos;
    refreshLine(l);
  }
}

/* Move cursor to the start of the line. */
static void linenoiseEditMoveHome(linenoiseState& l)
{
  if (l.pos != 0)
  {
    l.pos = 0;
    refreshLine(l);
  }
}

/* Move cursor to the end of the line. */
static void linenoiseEditMoveEnd(linenoiseState& l)
{
  if (l.pos != l.len)
  {
    l.pos = l.len;
    refreshLine(l);
  }
}

/* Substitute the currently edited line with the next or previous history
 * entry as specified by 'dir'. */
#define LINENOISE_HISTORY_NEXT 0
#define LINENOISE_HISTORY_PREV 1
static void linenoiseEditHistoryNext(linenoiseState& l, int dir)
{
  const auto size = history_container.size();

  if (size < 2)
  {
    return;
  }

  // Update the current history entry before overwriting it with the next one.
  history_container.back() = std::u32string(l.buf, l.len);

  // Show the next entry.
  l.history_index += dir == LINENOISE_HISTORY_PREV ? 1 : -1;
  if (l.history_index < 0)
  {
    l.history_index = 0;
  }
  else if (l.history_index >= static_cast<int>(size))
  {
    l.history_index = size - 1;
  } else {
    const auto& entry = history_container[size - 1 - l.history_index];

    std::memcpy(
      static_cast<void*>(l.buf),
      static_cast<const void*>(entry.c_str()),
      sizeof(char32_t) * std::min(l.buflen, entry.length())
    );
    l.buf[l.buflen - 1] = '\0';
    l.len = l.pos = entry.length();
    refreshLine(l);
  }
}

/* Delete the character at the right of the cursor without altering the cursor
 * position. Basically this is what happens with the "Delete" keyboard key. */
static void linenoiseEditDelete(linenoiseState& l)
{
  if (l.len > 0 && l.pos < l.len)
  {
    std::memmove(
      static_cast<void*>(l.buf + l.pos),
      static_cast<const void*>(l.buf + l.pos + 1),
      sizeof(char32_t) * (l.len - l.pos - 1)
    );
    --l.len;
    --l.buf[l.len] = '\0';
    refreshLine(l);
  }
}

/* Backspace implementation. */
static void linenoiseEditBackspace(linenoiseState& l)
{
  if (l.pos > 0 && l.len > 0)
  {
    std::memmove(
      static_cast<void*>(l.buf + l.pos - 1),
      static_cast<const void*>(l.buf + l.pos),
      sizeof(char32_t) * (l.len - l.pos)
    );
    --l.pos;
    --l.len;
    l.buf[l.len] = '\0';
    refreshLine(l);
  }
}

/**
 * Delete the previosu word, maintaining the cursor at the start of the
 * current word.
 */
static void linenoiseEditDeletePrevWord(linenoiseState& l)
{
  auto old_pos = l.pos;
  std::size_t diff;

  while (l.pos > 0 && l.buf[l.pos - 1] == ' ')
  {
    --l.pos;
  }
  while (l.pos > 0 && l.buf[l.pos - 1] != ' ')
  {
    --l.pos;
  }
  diff = old_pos - l.pos;
  std::memmove(
    static_cast<void*>(l.buf + l.pos),
    static_cast<const void*>(l.buf + old_pos),
    sizeof(char32_t) * (l.len - old_pos + 1)
  );
  l.len -= diff;
  refreshLine(l);
}

/* This function is the core of the line editing capability of linenoise.
 * It expects 'fd' to be already in "raw mode" so that every key pressed
 * will be returned ASAP to read().
 *
 * The resulting string is put into 'buf' when the user type enter, or
 * when ctrl+d is typed.
 *
 * The function returns the length of the current buffer. */
static int linenoiseEdit(int stdin_fd,
                         int stdout_fd,
                         char32_t* buf,
                         std::size_t buflen,
                         const std::string& prompt)
{
  linenoiseState l;

  /* Populate the linenoise state that we pass to functions implementing
   * specific editing functionalities. */
  l.ifd = stdin_fd;
  l.ofd = stdout_fd;
  l.buf = buf;
  l.buflen = buflen;
  l.prompt = prompt;
  l.oldpos = l.pos = 0;
  l.len = 0;
  l.cols = getColumns(stdin_fd, stdout_fd);
  l.maxrows = 0;
  l.history_index = 0;

  /* Buffer starts empty. */
  l.buf[0] = '\0';
  l.buflen--; /* Make sure there is always space for the nulterm */

  /* The latest history entry is always our current buffer, that
   * initially is just an empty string. */
  plorth::cli::terminal::add_to_history(std::u32string());

  if (write(l.ofd, l.prompt.c_str(), l.prompt.length()) == -1)
  {
    return -1;
  }

  for (;;)
  {
    unsigned char c;
    int nread;
    char seq[3];

    if ((nread = read(l.ifd, &c, 1)) <= 0)
    {
      return l.len;
    }

    switch(c) {
    case ENTER:    /* enter */
      if (!history_container.empty())
      {
        history_container.pop_back();
      }
      return (int)l.len;
    case CTRL_C:     /* ctrl-c */
      errno = EAGAIN;
      return -1;
    case BACKSPACE:   /* backspace */
    case 8:     /* ctrl-h */
      linenoiseEditBackspace(l);
      break;
    case CTRL_D:     /* ctrl-d, remove char at right of cursor, or if the
                        line is empty, act as end-of-file. */
      if (l.len > 0) {
        linenoiseEditDelete(l);
      } else {
        if (!history_container.empty())
        {
          history_container.pop_back();
        }
        return -1;
      }
      break;
    case CTRL_T:    /* ctrl-t, swaps current character with previous. */
      if (l.pos > 0 && l.pos < l.len)
      {
        int aux = buf[l.pos-1];

        buf[l.pos-1] = buf[l.pos];
        buf[l.pos] = aux;
        if (l.pos != l.len-1)
        {
          l.pos++;
        }
        refreshLine(l);
      }
      break;
    case CTRL_B:     /* ctrl-b */
      linenoiseEditMoveLeft(l);
      break;
    case CTRL_F:     /* ctrl-f */
      linenoiseEditMoveRight(l);
      break;
    case CTRL_P:    /* ctrl-p */
      linenoiseEditHistoryNext(l, LINENOISE_HISTORY_PREV);
      break;
    case CTRL_N:    /* ctrl-n */
      linenoiseEditHistoryNext(l, LINENOISE_HISTORY_NEXT);
      break;
    case ESC:    /* escape sequence */
      /* Read the next two bytes representing the escape sequence.
       * Use two calls to handle slow terminals returning the two
       * chars at different times. */
      if (read(l.ifd,seq,1) == -1) break;
      if (read(l.ifd,seq+1,1) == -1) break;

      /* ESC [ sequences. */
      if (seq[0] == '[') {
        if (seq[1] >= '0' && seq[1] <= '9') {
          /* Extended escape, read additional byte. */
          if (read(l.ifd,seq+2,1) == -1) break;
          // Delete key.
          if (seq[1] == '3' && seq[2] == '~')
          {
            linenoiseEditDelete(l);
          }
        } else {
          switch(seq[1]) {
            case 'A': /* Up */
              linenoiseEditHistoryNext(l, LINENOISE_HISTORY_PREV);
              break;
            case 'B': /* Down */
              linenoiseEditHistoryNext(l, LINENOISE_HISTORY_NEXT);
              break;
            case 'C': /* Right */
              linenoiseEditMoveRight(l);
              break;
            case 'D': /* Left */
              linenoiseEditMoveLeft(l);
              break;
            case 'H': /* Home */
              linenoiseEditMoveHome(l);
              break;
            case 'F': /* End*/
              linenoiseEditMoveEnd(l);
              break;
          }
        }
      }

      /* ESC O sequences. */
      else if (seq[0] == 'O') {
        switch(seq[1]) {
          case 'H': /* Home */
            linenoiseEditMoveHome(l);
            break;
          case 'F': /* End*/
            linenoiseEditMoveEnd(l);
            break;
        }
      }
      break;

    default:
      if (!linenoiseEditInsertUTF8(l, c))
      {
        return -1;
      }
      break;

    case CTRL_U: /* Ctrl+u, delete the whole line. */
      buf[0] = '\0';
      l.pos = l.len = 0;
      refreshLine(l);
      break;
    case CTRL_K: /* Ctrl+k, delete from current to end of line. */
      buf[l.pos] = '\0';
      l.len = l.pos;
      refreshLine(l);
      break;
    case CTRL_A: /* Ctrl+a, go to the start of the line */
      linenoiseEditMoveHome(l);
      break;
    case CTRL_E: /* ctrl+e, go to the end of the line */
      linenoiseEditMoveEnd(l);
      break;
    case CTRL_L: /* ctrl+l, clear screen */
      linenoiseClearScreen();
      refreshLine(l);
      break;
    case CTRL_W: /* ctrl+w, delete previous word */
      linenoiseEditDeletePrevWord(l);
      break;
    }
  }

  return l.len;
}

/**
 * This function calls the line editing function linenoiseEdit() using
 * the STDIN file descriptor set in raw mode.
 */
static int linenoiseRaw(char32_t* buf,
                        std::size_t buflen,
                        const std::string& prompt)
{
  int count;

  if (buflen == 0)
  {
    errno = EINVAL;

    return -1;
  }
  else if (enableRawMode(STDIN_FILENO) == -1)
  {
    return -1;
  }
  count = linenoiseEdit(STDIN_FILENO, STDOUT_FILENO, buf, buflen, prompt);
  disableRawMode(STDIN_FILENO);
  std::printf("\n");

  return count;
}

/**
 * This function is called when linenoise() is called with the standard
 * input file descriptor not attached to a TTY. So for example when the
 * program using linenoise is called in pipe or with a file redirected
 * to its standard input. In this case, we want to be able to return the
 * line regardless of its length (by default we are limited to 4k).
 */
static bool linenoiseNoTTY(std::u32string& result)
{
  std::string line;

  for (;;)
  {
    const auto c = std::fgetc(stdin);

    if (c == EOF || c == '\n')
    {
      if (c == EOF && line.empty())
      {
        return false;
      }
      result = plorth::utf8_decode(line);

      return true;
    }
    line.append(1, static_cast<char>(c));
  }
}

namespace plorth
{
  namespace cli
  {
    namespace terminal
    {
      /**
       * The high level function that is the main API of the linenoise library.
       * This function checks if the terminal has basic capabilities, just
       * checking for a blacklist of stupid terminals, and later either calls
       * the line editing function or uses dummy fgets() so that you will be
       * able to type something even in the most desperate of the conditions.
       */
      bool prompt(const std::string& prompt, std::u32string& result)
      {
        if (!::isatty(STDIN_FILENO))
        {
          // Not a tty: read from file / pipe. In this mode we don't want any
          // limit to the line size, so we call a function to handle that.
          return linenoiseNoTTY(result);
        }
        else if (isUnsupportedTerm())
        {
          char buffer[LINENOISE_MAX_LINE];
          std::size_t len;

          std::printf("%s", prompt.c_str());
          std::fflush(stdout);
          if (!std::fgets(buffer, LINENOISE_MAX_LINE, stdin))
          {
            return false;
          }
          len = std::strlen(buffer);
          while (len && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
          {
            buffer[--len] = '\0';
          }
          result = utf8_decode(buffer);
        } else {
          char32_t buffer[LINENOISE_MAX_LINE];
          const auto len = linenoiseRaw(buffer, LINENOISE_MAX_LINE, prompt);

          if (len == -1)
          {
            return false;
          }
          result.assign(buffer, len);
        }

        return true;
      }

      /**
       * This is the API call to add a new entry in the linenoise history.
       * It uses a fixed array of char pointers that are shifted (memmoved)
       * when the history max length is reached in order to remove the older
       * entry and make room for the new one, so it is not exactly suitable for
       * huge histories, but will work well for a few hundred of entries.
       *
       * Using a circular buffer is smarter, but a bit more complex to handle.
       */
      void add_to_history(const std::u32string& line)
      {
        if (history_max_len == 0)
        {
          return;
        }

        // Don't add duplicated lines.
        if (!history_container.empty()
            && !history_container.back().compare(line))
        {
          return;
        }

        if (history_container.size() == history_max_len)
        {
          history_container.pop_front();
        }

        history_container.push_back(line);
      }

      /**
       * Set the maximum length for the history. This function can be called
       * even if there is already some history, the function will make sure to
       * retain just the latest 'len' elements if the new history length value
       * is smaller than the amount of items already inside the history.
       */
      void set_history_max_size(std::size_t size)
      {
        if (size == 0)
        {
          history_container.clear();
        } else {
          while (history_container.size() > size)
          {
            history_container.pop_front();
          }
        }
        history_max_len = size;
      }
    }
  }
}

/* At exit we'll try to fix the terminal to the initial conditions. */
static void linenoiseAtExit()
{
  disableRawMode(STDIN_FILENO);
}
