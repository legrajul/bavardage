 /*
 * Client.vala
 * 
 * Copyright 2013 Charles Ango, Julien Legras
 * 
 */
using Gtk;
using Gee;
using Bavardage.ClientCore;
using Bavardage.Common;

namespace Bavardage {
    public struct Message {
        int code;
        uint8 sender[64];
        uint8 content[512];
        uint8 receiver[64];
    }
    
    public class Client: Gtk.Application {
        private Gtk.Builder builder;
        private static HashMap<string, ListStore> rooms_map_users = new HashMap<string, ListStore> ();
        private HashMap<string, TextBuffer> rooms_map_chats = new HashMap<string, TextBuffer> ();
        private HashMap<string, EntryBuffer> rooms_map_entries = new HashMap<string, EntryBuffer> ();
        private Gtk.Window window;
        private TreeView open_rooms;
        private TreeView connected_users;
        private TextView chat;
        private Entry message;
        private Button create_room_button;
        private Button quit_room_button;
        private Button join_room_button;
        private Gtk.MenuItem connect_item;
        private Gtk.MenuItem disconnect_item;
        private Gtk.MenuItem quit_item;
        private Gtk.MenuItem about_item;

        private string command_line = "";

        private signal void update_connected (bool is_connected);

        /*
         * Constructeur d'un client
         */
        public Client (string cl) {
            try {
                // On commence par définir notre Application
                Object(application_id: "bavardage.client",
                       flags: ApplicationFlags.HANDLES_OPEN);
                GLib.Environment.set_prgname("bavardage-client");
                this.register ();

                setup_ui (cl);
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
        private void setup_ui (string cl) {
            try {
                builder = new Builder ();
                cl.data[cl.length - 6] = '\0';
                builder.add_from_file (cl + "interface.ui");
                builder.connect_signals (this);
                window = builder.get_object ("mainWindow") as Window;
                connected_users = builder.get_object ("connected_users") as TreeView;
                connected_users.set_model (new ListStore (1, typeof (string)));
                connected_users.insert_column_with_attributes (-1, "Contacts connectés", new CellRendererText (), "text", 0);
                open_rooms = builder.get_object ("open_rooms") as TreeView;
                open_rooms.set_model (new ListStore (1, typeof (string)));
                open_rooms.insert_column_with_attributes (-1, "Salons ouverts", new CellRendererText (), "text", 0);
				
                chat = builder.get_object ("chat_view") as TextView;
                message = builder.get_object ("message_entry") as Entry;

                create_room_button = builder.get_object ("button_create_room") as Button;
                quit_room_button = builder.get_object ("button_quit_room") as Button;
                join_room_button = builder.get_object ("button_join_room") as Button;
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
                    if (disconnect_item.get_sensitive ()) {
                        ClientCore.disconnect ();
                    }
                    Gtk.main_quit ();
                });

            // On clique sur Fichier > Quitter
            quit_item.activate.connect ( () => {
                    // se déconnecter du serveur
                    if (disconnect_item.get_sensitive ()) {
                        ClientCore.disconnect ();
                    }
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

                            if (((string) v)[0] == '[') {
                                connected_users.get_parent ().hide ();
                            } else {
                                connected_users.get_parent ().show_all ();
                            }
                        }
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
                    var select = open_rooms.get_selection ();
                    if (select.get_selected (out m, out iter)) {
                        Value v;
                        m.get_value (iter, 0, out v);
                        string s = (string) v;
                        if (s[0] != '[') {
                            send_message ("/QUIT_ROOM " + s);
                        } else {
                            Value v2;
                            var model2 = open_rooms.get_model () as ListStore;
                            model2.get_iter_first (out iter);
                            do {
                                model2.get_value (iter, 0, out v2);
                                if (s == (string) v2) {
                                    break;
                                }
                            } while (model2.iter_next (ref iter));
                            model2.remove (iter);
                            rooms_map_chats.unset (s);
                            rooms_map_entries.unset (s); 
                            rooms_map_users.unset (s); 
                        }
                    }
                });

            join_room_button.clicked.connect ( () => {
                    var dialog = new Dialog.with_buttons ("Rejoindre un salon un salon", window, DialogFlags.MODAL | DialogFlags.DESTROY_WITH_PARENT, Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.OK, Gtk.ResponseType.ACCEPT);
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
                                send_message ("/JOIN_ROOM " + entry_room_name.get_text ());
                            }
                            dialog.hide_on_delete ();
                        });
                    dialog.show_all ();
                });

            message.activate.connect ( () => {
                    send_message_entry ();
                });
            
            // On clique sur le bouton "Envoyer"
            message.icon_press.connect ( (p0, p1) => {
                    send_message_entry ();
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
                                Thread<void *> t = new Thread<void *>.try ("recv thread", this.receive_thread);
                                Thread.usleep (10000);
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
                    ClientCore.disconnect ();
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

                    dialog.license = "Logiciel développé dans le cadre d'un projet universitaire encadré par Magali Bardet";
                    dialog.wrap_license = true;

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


        private void *receive_thread () {
            Message m = { -1, "".data, "".data, "".data };
            TreeIter tree_iter;
            while (true) {
                m = { -1, "".data, "".data, "".data };
                if (receive_message (out m) == 0) {
                    var sender = new StringBuilder ("");
                    for (int i = 0; i < m.sender.length; i++) {
                        sender.append_c((char) m.sender[i]);
                    }
                    var content = new StringBuilder ("");
                    for (int i = 0; i < m.content.length; i++) {
                        content.append_c ((char) m.content[i]);
                    }
                    var receiver = new StringBuilder ("");
                    for (int i = 0; i < m.receiver.length; i++) {
                        receiver.append_c ((char) m.receiver[i]);
                    }
                    switch (m.code) {
                    case KO:
                        var dialog =  new Gtk.MessageDialog (window, Gtk.DialogFlags.MODAL, Gtk.MessageType.WARNING, Gtk.ButtonsType.OK, content.str);
                        dialog.response.connect ((response_id) => {
                                dialog.destroy ();
                            });
                        dialog.show ();
                        stdout.printf ("Error: %s\n", content.str);
                        break;
                    case OK:
                        
                        break;
                    case CREATE_ROOM:
                        var rooms = open_rooms.get_model () as ListStore;
                        rooms.append (out tree_iter);
                        rooms.set (tree_iter, 0, content.str, -1);

                        rooms_map_users.set (content.str, new ListStore (1, typeof (string)));
                        rooms_map_chats.set (content.str, new TextBuffer (new TextTagTable ()));
                        rooms_map_entries.set (content.str, new EntryBuffer ("".data));
                        
                        break;
                    case DELETE_ROOM:
                        Value v;
                        var model = open_rooms.get_model () as ListStore;
                        model.get_iter_first (out tree_iter);
                        do {
                            model.get_value (tree_iter, 0, out v);
                            if (content.str == (string) v) {
                                break;
                            }
                        } while (model.iter_next (ref tree_iter));
                        model.remove (tree_iter);
                        rooms_map_chats.unset (content.str);
                        rooms_map_entries.unset (content.str); 
                        rooms_map_users.unset (content.str); 
                        break;
                    case MESSAGE:
                        string s = "<" + sender.str + "> " + content.str + "\n";
                        TextIter iter;
                        rooms_map_chats.get (receiver.str).get_end_iter (out iter);
                        rooms_map_chats.get (receiver.str).insert_text (ref iter, s, s.length);
                        break;

                    case MP:
                        string s = "[" + sender.str + "] " + content.str + "\n";
                        TextIter iter;
                        string room_name = "[" + sender.str + "]";
                        if (sender.str == ClientCore.get_login ()) {
                            room_name = "[" + receiver.str + "]";
                        }
                        if (rooms_map_chats.get (room_name) == null ) {
                            var rooms = open_rooms.get_model () as ListStore;
                            rooms.append (out tree_iter);
                            rooms.set (tree_iter, 0, room_name, -1);

                            rooms_map_users.set (room_name, new ListStore (1, typeof (string)));
                            rooms_map_chats.set (room_name, new TextBuffer (new TextTagTable ()));
                            rooms_map_entries.set (room_name, new EntryBuffer ("".data));
                        }
                        rooms_map_chats.get (room_name).get_end_iter (out iter);
                        rooms_map_chats.get (room_name).insert_text (ref iter, s, s.length);
                        break;
                    case NEW_USER:
                        string s = sender.str + " vient de rejoindre le salon\n";
                        TextIter iter;
                        string room_name = content.str;
                        rooms_map_chats.get (room_name).get_end_iter (out iter);
                        rooms_map_chats.get (room_name).insert_text (ref iter, s, s.length);
                        var users = rooms_map_users.get (content.str) as ListStore;
                        users.append (out tree_iter);
                        users.set (tree_iter, 0, sender.str, -1);
                        break;
                    case ADD_USER:
                        var users = rooms_map_users.get (content.str) as ListStore;
                        users.append (out tree_iter);
                        users.set (tree_iter, 0, sender.str, -1);
                        break;
                    case RM_USER:
                        string s = sender.str + " vient de quitter le salon\n";
                        TextIter iter;
                        string room_name = content.str;
                        rooms_map_chats.get (room_name).get_end_iter (out iter);
                        rooms_map_chats.get (room_name).insert_text (ref iter, s, s.length);
                        Value v;
                        var model = rooms_map_users.get (content.str)as ListStore;
                        model.get_iter_first (out tree_iter);
                        do {
                            model.get_value (tree_iter, 0, out v);
                            if (sender.str == (string) v) {
                                break;
                            }
                        } while (model.iter_next (ref tree_iter));
                        model.remove (tree_iter);
                        
                        break;
                    default:
                        break;
                    }
                } else {
                    stdout.printf ("Error\n");
                    break;
                }
            }
            Thread.exit (null);
            return null;
        }

        private void send_message_entry () {
            stdout.printf ("send_message_entry ()\n");
            var msg = message.get_text ();
            stdout.printf ("message à envoyer : %s\n", msg);

            if (msg[0] != '/') {
                TreeModel m;
                TreeIter iter;
                var select = open_rooms.get_selection ();
                if (select.get_selected (out m, out iter)) {
                    Value v;
                    m.get_value (iter, 0, out v);
                    if (((string) v)[0] == '[') {
                        StringBuilder recv_name = new StringBuilder ("");
                        for (int i = 1; i < ((string) v).length - 1; i++) {
                            recv_name.append_c ((char) ((string) v).data[i]);
                        }
                        send_message ("/MP " + recv_name.str + " " + msg);
                    } else {
                        send_message ("/MESSAGE " + (string) v + " " + msg);
                    }
                    
                }
            } else {
                send_message (msg);
            }
            message.set_text("");
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

        }

    }

    /*
     * Point d'entrée du programme
     */
    int main (string[] args) {
        Gtk.init (ref args);
        new Bavardage.Client (args[0]);
        Gtk.main ();
        return 0;
    }

}

