#include <plorth/gui/window.hpp>

int main(int argc, char** argv)
{
  auto app = Gtk::Application::create(argc, argv, "org.plorth.gui");
  plorth::gui::Window window;

  return app->run(window);
}
