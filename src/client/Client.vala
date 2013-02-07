 /*
 * Client.vala
 * 
 * Copyright 2013 Charles Ango, Julien Legras
 * 
 */
using Gtk;
using Gee;
using Bavardage.ClientCore;

namespace Bavardage {
	public class Client: Gtk.Application {
		private Gtk.Builder builder;
		private static HashMap<string, ListStore> rooms_map_users = new HashMap<string, ListStore> ();
		private HashMap<string, TextBuffer> rooms_map_chats = new HashMap<string, TextBuffer> ();
		private HashMap<string, EntryBuffer> rooms_map_entries = new HashMap<string, EntryBuffer> ();
		private Gtk.Window window;
		private TreeView open_rooms;
		private TreeView invited_rooms;
		private TreeView connected_users;
		private TextView chat;
		private Entry message;
		private Button create_room_button;
		private Button quit_room_button;
		private Button send_button;
		private Gtk.MenuItem connect_item;
		private Gtk.MenuItem disconnect_item;
		private Gtk.MenuItem quit_item;
		private Gtk.MenuItem about_item;

		private signal void update_connected (bool is_connected);

		/*
		 * Constructeur d'un client
		 */
		public Client () {
			try {
				// On commence par définir notre Application
				Object(application_id: "bavardage.client",
					flags: ApplicationFlags.HANDLES_OPEN);
				GLib.Environment.set_prgname("bavardage-client");
				this.register ();

				setup_ui ();
				connect_signals ();
				update_connected (false);
				window.show_all ();
				connect_item.activate ();
			} catch (Error e) {
				stderr.printf ("Error: %s\n", e.message);
			}
		}

		/*
		 * Fonction qui charge l'interface et initialise les attributs d'interface
		 */
		private void setup_ui () {
			try {
				builder = new Builder ();
				builder.add_from_file ("interface.ui");
				builder.connect_signals (this);
				window = builder.get_object ("mainWindow") as Window;
				connected_users = builder.get_object ("connected_users") as TreeView;
				connected_users.set_model (new ListStore (1, typeof (string)));
				connected_users.insert_column_with_attributes (-1, "Contacts connectés", new CellRendererText (), "text", 0);
				open_rooms = builder.get_object ("open_rooms") as TreeView;
				open_rooms.set_model (new ListStore (1, typeof (string)));
				open_rooms.insert_column_with_attributes (-1, "Salons ouverts", new CellRendererText (), "text", 0);
				invited_rooms = builder.get_object ("invited_rooms") as TreeView;
				invited_rooms.set_model (new ListStore (1, typeof (string)));
				invited_rooms.insert_column_with_attributes (-1, "Invitations", new CellRendererText (), "text", 0);
				
				chat = builder.get_object ("chat_view") as TextView;
				message = builder.get_object ("message_entry") as Entry;

				create_room_button = builder.get_object ("button_create_room") as Button;
				quit_room_button = builder.get_object ("button_quit_room") as Button;
				send_button = builder.get_object ("send_button") as Button;
				connect_item = builder.get_object ("imagemenuitem2") as Gtk.MenuItem;
				disconnect_item = builder.get_object ("imagemenuitem1") as Gtk.MenuItem;
				quit_item = builder.get_object ("imagemenuitem5") as Gtk.MenuItem;
				about_item = builder.get_object ("imagemenuitem10") as Gtk.MenuItem;

				
				window.set_application (this);
			} catch (Error e) {
			 	stderr.printf ("Could not load UI: %s\n", e.message);
			}
		}

		/*
		 * Fonction qui connecte les signaux de l'interface
		 */
		private void connect_signals () {
			// On clique sur la croix de la fenêtre
			window.destroy.connect ( () => {
				// se déconnecter du serveur
				Gtk.main_quit ();
			});

			// On clique sur Fichier > Quitter
			quit_item.activate.connect ( () => {
				// se déconnecter du serveur
				Gtk.main_quit ();
			});

			// On clique sur un salon ouvert (<=> on change de salon)
			open_rooms.cursor_changed.connect ( () => {
				TreeModel m;
				TreeIter iter;
				var select = open_rooms.get_selection ();
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
			});
			
			
			// On clique sur une invitation, demande une confirmation
			invited_rooms.cursor_changed.connect ( () => {
				TreeModel m;
				TreeIter iter;
				var select = invited_rooms.get_selection ();
				if (select.get_selected (out m, out iter)) {
					Value v;
					m.get_value (iter, 0, out v);
					var dialog = new Gtk.MessageDialog (window, Gtk.DialogFlags.MODAL, Gtk.MessageType.QUESTION, Gtk.ButtonsType.YES_NO, "Voulez-vous rejoindre le salon %s ?", (string) v);
					dialog.response.connect ( (response_id) => {
						if (response_id == Gtk.ResponseType.YES) {
							var list_rooms = open_rooms.get_model () as ListStore;
							TreeIter iter2;
							list_rooms.append (out iter2);
							list_rooms.set (iter2, 0, (string) v);

							var list_invitations = invited_rooms.get_model () as ListStore;
							select.set_mode (Gtk.SelectionMode.NONE);
							list_invitations.remove (iter);
							select.set_mode (Gtk.SelectionMode.SINGLE);
						}
						dialog.destroy ();
					});
					dialog.show ();
				}
			});

			// On clique sur le bouton "Créer un salon"
			create_room_button.clicked.connect ( () => {
				var dialog = new Dialog.with_buttons ("Création d'un salon", window, DialogFlags.MODAL | DialogFlags.DESTROY_WITH_PARENT, Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.OK, Gtk.ResponseType.ACCEPT);
				var content = dialog.get_content_area () as Gtk.Box;
				var grid = new Gtk.Grid ();
				var label = new Gtk.Label ("Nom du salon :");
				grid.attach (label, 0, 0, 1, 1);
				var entry_room_name = new Gtk.Entry ();
				grid.attach (entry_room_name, 1, 0, 1, 1);
				
				content.add (grid);
				dialog.response.connect ((response_id) => {
					if (response_id == Gtk.ResponseType.ACCEPT) {
						// demander à créer le salon
                        send_message ("/CREATE_ROOM " + entry_room_name.get_text ());
					}
                    dialog.hide_on_delete ();
				});
				dialog.show_all ();
			});

			// On clique sur le bouton "Quitter le salon"
			quit_room_button.clicked.connect ( () => {
				TreeModel m;
				TreeIter iter;
				ListStore list;
				Value v;
				var select = open_rooms.get_selection ();
				if (select.get_selected (out m, out iter)) {
					m.get_value (iter, 0,out v); // On récupère le nom du salon qu'on a choisi
					var salon = (string) v; 
					if (salon != "accueil") {	// on vérifie que ce n'est pas le salon principal
					list = (m) as ListStore;	// on cast le Model reçu en listStore
					list.remove(iter);			// On supprime le salon de la liste des salons
					}
				}
			});

			// On clique sur le bouton "Envoyer"
			send_button.clicked.connect ( () => {
				var msg = message.get_text ();
				stdout.printf ("message à envoyer : %s\n", msg);
                send_message (msg);
				message.set_text("");
			});
			
			// On appuie sur la touche "Entrée"
			message.key_press_event.connect( (e)=> {
				//e = new Gdk.Event(Gdk.EventType.KEY_PRESS);
				if (e.keyval == Gdk.Key.Return){
					var msg = message.get_text();
					stdout.printf ("message à envoyer: %u\n",e.keyval);
					stdout.printf ("message à envoyer: %s\n", msg);
					message.set_text("");
					return true;
				} else {
					return false;
				}
			});
			
			// On clique sur Fichier > Connexion
			connect_item.activate.connect ( () => {
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
						connect_socket (entry_server_ip.get_text (), int.parse (entry_server_port.get_text ()));
						send_message ("/CONNECT " + entry_login.get_text ());
						update_connected (true);
						dialog.hide_on_delete ();
					}
				});
				dialog.show_all ();
			});

			// On clique sur Fichier > Déconnexion
			disconnect_item.activate.connect ( () => {
				// démander déconnexion
					update_connected (false);
			});

			// On clique sur Fichier > À propos
			about_item.activate.connect ( () => {
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
			});

			// Écoute les changements d'état connecté/déconnecté
			this.update_connected.connect ( (is_connected) => {
				connect_item.set_sensitive (!is_connected);
				disconnect_item.set_sensitive (is_connected);
			});
		}

		/*
		 * Fonction qui met des valeurs dans les différents éléments
		 * graphiques pour tester l'application
		 */
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
			rooms.append (out iter);
			rooms.set (iter, 0, "accueil", -1);
			rooms_view.set_model (rooms);
			
			rooms_map_users.set ("salon 1", users_room1);
			rooms_map_users.set ("salon 2", users_room2);
			rooms_map_users.set ("accueil", new ListStore (1, typeof (string)));

			var buffer1 = new TextBuffer (new TextTagTable ());
			buffer1.set_text ("Texte du salon 1...");
			var buffer2 = new TextBuffer (new TextTagTable ());
			buffer2.set_text ("Texte du salon 2...");

			rooms_map_chats.set ("salon 1", buffer1);
			rooms_map_chats.set ("salon 2", buffer2);
			rooms_map_chats.set ("accueil", new TextBuffer (new TextTagTable ()));

			var entry1 = new EntryBuffer ("Message 1...".data);
			var entry2 = new EntryBuffer ("Message 2...".data);

			rooms_map_entries.set ("salon 1", entry1);
			rooms_map_entries.set ("salon 2", entry2);
			rooms_map_entries.set ("accueil", new EntryBuffer ({}));

			var invitations = invited_rooms.get_model () as ListStore;
			invitations.append (out iter);
			invitations.set (iter, 0, "Super salon");
			invitations.append (out iter);
			invitations.set (iter, 0, "Table ronde");
		}
		
	}

	/*
	 * Point d'entrée du programme
	 */
	int main (string[] args) {
		Gtk.init (ref args);
		var c = new Bavardage.Client ();
		//c.setup_test ();
		Gtk.main ();
		return 0;
	}

}

