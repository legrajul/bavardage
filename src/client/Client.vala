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

		
		public Client () {
			try {
				// On commence par définir notre Application
				Object(application_id: "bavardage.client",
					flags: ApplicationFlags.FLAGS_NONE);
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
				var dialog = builder.get_object ("connection_dialog") as Dialog;
				dialog.show_all ();
			
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
