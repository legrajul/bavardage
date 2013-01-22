using Gtk;

namespace Bavardage {
	public class Client: Gtk.Application {

		[CCode (instance_pos = -1)]
		public void on_send_button_clicked (Button source) {
			stdout.printf ("Test\n");
		}

		[CCode (instance_pos = -1)]
		public void on_destroy (Widget window) {
			Gtk.main_quit();
		}
		
		public Client () {
			try {
				// On commence par définir notre Application
				Object(application_id: "bavardage.client",
					flags: ApplicationFlags.FLAGS_NONE);
				GLib.Environment.set_prgname("bavardage-client");
				// Puis on peut l'enregistrer (permet l'unicité)
				this.register ();

				// On charge l'interface depuis le fichier xml
				var builder = new Builder ();
				builder.add_from_file ("interface.ui");
				builder.connect_signals (this);
				var window = builder.get_object ("mainWindow") as Window;
				window.set_application (this);
				window.show_all ();
				var dialog = builder.get_object ("connection_dialog") as Dialog;
				dialog.show_all ();
			
			} catch (Error e) {
				stderr.printf ("Could not load UI: %s\n", e.message);
			}
		}

		
	}
}

int main (string[] args) {     
    Gtk.init (ref args);
	new Bavardage.Client (); 
	Gtk.main ();
    return 0;
}
