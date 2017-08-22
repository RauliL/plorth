var Module = { noExitRuntime: true };

(() => {
  const promptContainer = document.createElement('div');
  const promptLabel = document.createElement('span');
  const promptInput = document.createElement('input');
  const lineContainer = document.createElement('ul');
  let lineCounter = 1;

  const updatePrompt = () => {
    promptLabel.textContent = `plorth:${lineCounter}:${Module.depth ? Module.depth() : 0}>`;
  };

  Module.print = (text, className) => {
    const line = document.createElement('li');

    line.textContent = text;
    if (className) {
      line.className = className;
    }
    lineContainer.appendChild(line);
  };

  Module.printErr = (text) => Module.print(text, 'error');

  promptContainer.className = 'prompt';
  promptContainer.appendChild(promptLabel);
  promptContainer.appendChild(promptInput);
  document.body.appendChild(lineContainer);
  document.body.appendChild(promptContainer);
  updatePrompt();
  promptInput.focus();

  promptInput.addEventListener('keydown', (ev) => {
    const input = promptInput.value.trim();

    if (ev.key !== 'Enter') {
      return;
    }
    ev.preventDefault();
    if (input.length > 0) {
      Module.print(`${promptLabel.textContent} ${input}`, 'user-input');
      Module.eval(input);
      ++lineCounter;
    }
    updatePrompt();
    promptInput.value = '';
    promptInput.focus();
  });
})();
