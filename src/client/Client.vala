using Gtk;
using Gee;


namespace Bavardage {
	public class Client: Gtk.Application {
		private static Gtk.Builder builder;
		private static HashMap<string, ListStore> rooms_map_users = new HashMap<string, ListStore> ();
		private static HashMap<string, TextBuffer> rooms_map_chats = new HashMap<string, TextBuffer> ();
		private static HashMap<string, EntryBuffer> rooms_map_entries = new HashMap<string, EntryBuffer> ();
		private static TreeView connected_users;
		private static TextView chat;
		private static Entry message;
		
		[CCode (instance_pos = -1)]
		public void on_send_button_clicked (Button source) {
			stdout.printf ("send_button_clicked\n");
		}
		
		
/*Signaux des boutons gerant les salons*/
		[CCode (instance_pos = -1)]
		public void on_button_create_room_clicked (Button source) {
			stdout.printf ("button_create_room_clicked\n");
		}
		
		[CCode (instance_pos = -1)]
		public void on_button_quit_room_clicked (Button source, TreeSelection select) {
			stdout.printf ("button_quit_room_clicked\n");
			TreeModel m;
			TreeIter iter;
			ListStore list;
			Value v;			
			if (select.get_selected (out m, out iter)) {
				m.get_value (iter, 0,out v); // On récupère le nom du salon qu'on a choisi
				var salon = (string) v; 
				if (salon != "salon 2") {	// on vérifie que ce n'est pas le salon principal
				list = (m) as ListStore;	// on cast le Model reçu en listStore
				list.remove(iter);			// On supprime le salon de la liste des salons
				}
			}
		}
		
		[CCode (instance_pos = -1)]
		public void on_button1_clicked (Button source) {
			stdout.printf ("button_1_clicked\n");
		}
		
		[CCode (instance_pos = -1)]
		public void on_touched_button (Window window, Gdk.Event e){
			if (e.type== Gdk.EventType.KEY_PRESS){
			var k= (uint) e.key.keyval;
			stdout.printf ("%u\n",k);
			}		
		}
/*Fin de la liste des signaux des boutons concernant les salons*/


		[CCode (instance_pos = -1)]
		public void on_destroy (Widget window) {
			Gtk.main_quit();
		}

		[CCode (instance_pos = -1)]
		public void room_changed (Widget tree_view, TreeSelection select) {
			
			TreeModel m;
			TreeIter iter;
			if (select.get_selected (out m, out iter)) {
				Value v;
				m.get_value (iter, 0, out v);
				if (rooms_map_users != null) {
					var users_model = rooms_map_users.get ((string) v) as TreeModel;
					connected_users.set_model (users_model);
					var chat_model = rooms_map_chats.get ((string) v) as TextBuffer;
					chat.set_buffer (chat_model);
					var entry_model = rooms_map_entries.get ((string) v) as EntryBuffer;
					message.set_buffer (entry_model);
				}
			}
		}


		[CCode (instance_pos = -1)]
		public void show_aboutdialog () {
			var window = builder.get_object ("mainWindow") as Window;
			var dialog = new AboutDialog ();
			dialog.set_destroy_with_parent (true);
			dialog.set_transient_for (window);
			dialog.set_modal (true);

			dialog.artists = {"Charles Ango", "Julien Legras"};
			dialog.authors = {"Charles Ango", "Ismaël Kabore", "Julien Legras", "Yves Nouafo", "Jean-Baptiste Souchal"};

			dialog.program_name = "Barvardage";
			dialog.comments = "Messagerie instantanée";
			dialog.copyright = "Copyright © 2012-2013";
			dialog.version = "0.1";


			dialog.website = "http://github.com/legrajul/bavardage";
			dialog.website_label = "Dépôt github";

			dialog.response.connect ((response_id) => {
				if (response_id == Gtk.ResponseType.CANCEL || response_id == Gtk.ResponseType.DELETE_EVENT) {
					dialog.hide_on_delete ();
				}
			});
			dialog.present ();
		}

		[CCode (instance_pos = -1)]
		public void show_connection_dialog () {
			var window = builder.get_object ("mainWindow") as Window;
			var dialog = new Dialog.with_buttons ("Connexion", window, DialogFlags.MODAL | DialogFlags.DESTROY_WITH_PARENT, Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.CONNECT, Gtk.ResponseType.ACCEPT);
			var content = dialog.get_content_area () as Gtk.Box;
			var grid = new Gtk.Grid ();
			var label = new Gtk.Label ("Adresse du serveur :");
			grid.attach (label, 0, 0, 1, 1);
			var entry_server_ip = new Gtk.Entry ();
			grid.attach (entry_server_ip, 1, 0, 1, 1);
			
			label = new Gtk.Label ("Port du serveur :");
			grid.attach (label, 0, 1, 1, 1);
			var entry_server_port = new Gtk.Entry ();
			grid.attach (entry_server_port, 1, 1, 1, 1);

			label = new Gtk.Label ("Pseudo :");
			grid.attach (label, 0, 2, 1, 1);
			var entry_login = new Gtk.Entry ();
			grid.attach (entry_login, 1, 2, 1, 1);

			content.add (grid);
			dialog.response.connect ((response_id) => {
				if (response_id == Gtk.ResponseType.CANCEL || response_id == Gtk.ResponseType.DELETE_EVENT) {
					dialog.hide_on_delete ();
				} else if (response_id == Gtk.ResponseType.ACCEPT) {
					// établir la connexion
				}
			});
			dialog.show_all ();
		}
		
		public Client () {
			try {
				// On commence par définir notre Application
				Object(application_id: "bavardage.client",
					flags: ApplicationFlags.HANDLES_OPEN);
				GLib.Environment.set_prgname("bavardage-client");
				// Puis on peut l'enregistrer (permet l'unicité)
				this.register ();

				// On charge l'interface depuis le fichier xml
				builder = new Builder ();
				builder.add_from_file ("interface.ui");
				builder.connect_signals (this);
				var window = builder.get_object ("mainWindow") as Window;
				connected_users = builder.get_object ("connected_users") as TreeView;
				connected_users.insert_column_with_attributes (-1, "Contacts connectés", new CellRendererText (), "text", 0);
				var rooms = builder.get_object ("open_rooms") as TreeView;
				rooms.insert_column_with_attributes (-1, "Salons ouverts", new CellRendererText (), "text", 0);
				var invited_rooms = builder.get_object ("invited_rooms") as TreeView;
				invited_rooms.insert_column_with_attributes (-1, "Invitations", new CellRendererText (), "text", 0);
				
				chat = builder.get_object ("chat_view") as TextView;
				message = builder.get_object ("message_entry") as Entry;
				window.set_application (this);
				window.show_all ();
				show_connection_dialog ();
			} catch (Error e) {
				stderr.printf ("Could not load UI: %s\n", e.message);
			}
		}

		public void setup_test () {

			var users_room1 = new ListStore (1, typeof (string));
			TreeIter iter;
			users_room1.append (out iter);
			users_room1.set (iter, 0, "Admin");
			users_room1.append (out iter);
			users_room1.set (iter, 0, "Titi");

			var users_room2 = new ListStore (1, typeof (string));
			users_room2.append (out iter);
			users_room2.set (iter, 0, "Admin");
			users_room2.append (out iter);
			users_room2.set (iter, 0, "Toto");
	
			var rooms_view = builder.get_object ("open_rooms") as TreeView;
			var rooms = new ListStore (1, typeof (string));
			rooms.append (out iter);
			rooms.set (iter, 0, "salon 1", -1);
			rooms.append (out iter);
			rooms.set (iter, 0, "salon 2", -1);
			rooms_view.set_model (rooms);
			
			rooms_map_users.set ("salon 1", users_room1);
			rooms_map_users.set ("salon 2", users_room2);

			var buffer1 = new TextBuffer (new TextTagTable ());
			buffer1.set_text ("Texte du salon 1...");
			var buffer2 = new TextBuffer (new TextTagTable ());
			buffer2.set_text ("Texte du salon 2...");

			rooms_map_chats.set ("salon 1", buffer1);
			rooms_map_chats.set ("salon 2", buffer2);

			var entry1 = new EntryBuffer ("Message 1...".data);
			var entry2 = new EntryBuffer ("Message 2...".data);

			rooms_map_entries.set ("salon 1", entry1);
			rooms_map_entries.set ("salon 2", entry2);
		}
		
	}
}

int main (string[] args) {
    Gtk.init (ref args);
	var c = new Bavardage.Client ();
	c.setup_test ();
	Gtk.main ();
    return 0;
}
