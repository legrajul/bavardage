/*
 * Client.vala
 *
 * Copyright 2013 Charles Ango, Julien Legras
 *
 */
using Gtk;
using Gee;
using Bavardage.ClientCore;
using Bavardage.ClientSecCore;
using Bavardage.Common;
using Bavardage.CommonSec;

namespace Bavardage {
    public struct Message {
        int code;
        uint8 sender[64];
        uint8 receiver[64];
        uint8 content[2048];
    }

    public class KeyIv {
        public uint8 key[32];
        public uint8 iv[32];
    }

    public class Client: Gtk.Application {
        private Gtk.Builder builder;
        public HashMap<string, ListStore> rooms_map_users = new HashMap<string, ListStore> ();
        public HashMap<string, TextBuffer> rooms_map_chats = new HashMap<string, TextBuffer> ();
        public HashMap<string, EntryBuffer> rooms_map_entries = new HashMap<string, EntryBuffer> ();
        public HashMap<string, KeyIv> secure_rooms_map_keyiv = new HashMap<string, KeyIv> ();
        public HashSet<string> secure_rooms = new HashSet<string> ();

        public string exec_directory { get; private set; }

        public Gtk.Window window;
        public TreeView open_rooms;
        public TreeView connected_users;
        public TextView chat;
        public Entry message;
        public Button create_room_button;
        public Button quit_room_button;
        public Button join_room_button;
        public Button send_mp_button;
        public Button invite_button;
        public Button kick_button;
        public Gtk.MenuItem connect_item;
        public Gtk.MenuItem disconnect_item;
        public Gtk.MenuItem delete_sec_account_item;
        public Gtk.MenuItem quit_item;
        public Gtk.MenuItem about_item;
        public Statusbar statusbar;
        public Statusbar secure_statusbar;
        public Bavardage.Widgets.ConnectionDialog conn_dial;
        public Thread<void *> thread_receive;
        public Thread<void *> thread_receive_sec;

        public bool is_secured = false;

        public signal void update_connected (bool is_connected, string login);

        /*
         * Constructeur d'un client
         */
        public Client (string cl) {
            cl.data[cl.length - 6] = '\0';
            ClientSecCore.set_root_certif_filename (cl + "root.pem");
            try {
                // On commence par définir notre Application
                Object(application_id: "bavardage.client",
                       flags: ApplicationFlags.HANDLES_OPEN);
                GLib.Environment.set_prgname("bavardage-client");
                this.register ();

                setup_ui (cl);
                connect_signals ();
                update_connected (false, "");
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
                exec_directory = cl;
                builder.add_from_file (cl + "interface.ui");
                builder.connect_signals (this);
                window = builder.get_object ("mainWindow") as Window;
                connected_users = builder.get_object ("connected_users") as TreeView;
                connected_users.set_model (new ListStore (2, typeof (string), typeof (string)));
                connected_users.insert_column_with_attributes (-1, "Contacts connectés", new CellRendererText (), "text", 0, null);
                connected_users.insert_column_with_attributes (-1, null, new CellRendererText (), "text", 1, null);
                open_rooms = builder.get_object ("open_rooms") as TreeView;
                open_rooms.set_model (new ListStore (2, typeof (string), typeof (Gdk.Pixbuf)));
                open_rooms.insert_column_with_attributes (-1, "Salons ouverts", new CellRendererText (), "text", 0, null);
                open_rooms.insert_column_with_attributes (-1, null, new CellRendererPixbuf (), "pixbuf", 1, null);

                chat = builder.get_object ("chat_view") as TextView;
                message = builder.get_object ("message_entry") as Entry;

                create_room_button = builder.get_object ("button_create_room") as Button;
                quit_room_button = builder.get_object ("button_quit_room") as Button;
                join_room_button = builder.get_object ("button_join_room") as Button;
                send_mp_button = builder.get_object ("button_send_mp") as Button;
                invite_button = builder.get_object ("invite_button") as Button;
                kick_button = builder.get_object ("kick_button") as Button;
                connect_item = builder.get_object ("imagemenuitem2") as Gtk.MenuItem;
                disconnect_item = builder.get_object ("imagemenuitem1") as Gtk.MenuItem;
                delete_sec_account_item = builder.get_object ("imagemenuitem3") as Gtk.MenuItem;
                quit_item = builder.get_object ("imagemenuitem5") as Gtk.MenuItem;
                about_item = builder.get_object ("imagemenuitem10") as Gtk.MenuItem;

                statusbar = builder.get_object ("statusbar") as Statusbar;
                statusbar.push (statusbar.get_context_id ("status"), "Déconnecté");
                secure_statusbar = builder.get_object ("secure_statusbar") as Statusbar;
                window.set_application (this);

                window.show_all ();

                conn_dial = new Bavardage.Widgets.ConnectionDialog (window, this);

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
                        if (is_secured) {
                            ClientSecCore.disconnect_servers ();
                        } else {
                            ClientCore.disconnect ();
                        }
                    }
                    Gtk.main_quit ();
                });

            // On clique sur Fichier > Quitter
            quit_item.activate.connect ( () => {
                    // se déconnecter du serveur
                    if (disconnect_item.get_sensitive ()) {
                        if (is_secured) {
                            ClientSecCore.disconnect_servers ();
                        } else {
                            ClientCore.disconnect ();
                        }
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
                            var users_model = rooms_map_users.get ((string) v) as ListStore;
                            connected_users.set_model (users_model);
                            var chat_model = rooms_map_chats.get ((string) v) as TextBuffer;
                            chat.set_buffer (chat_model);
                            var entry_model = rooms_map_entries.get ((string) v) as EntryBuffer;
                            message.set_buffer (entry_model);

                            TextIter text_iter;
                            chat_model.get_iter_at_line (out text_iter, chat_model.get_line_count ());
                            chat.scroll_to_iter (text_iter, 0.0, false, 0.0, 1.0);

                            if (((string) v)[0] == '[') {
                                connected_users.get_parent ().hide ();
                                send_mp_button.hide ();
                                invite_button.hide ();
                                kick_button.hide ();
                            } else {
                                connected_users.get_parent ().show_all ();
                                send_mp_button.show ();
                                invite_button.show ();
                                kick_button.show ();
                                
                            }
                            kick_button.set_sensitive (is_secure_room ((string) v));
                            invite_button.set_sensitive (is_secure_room ((string) v));
                        }
                    }
                });

            // On clique sur le bouton "Créer un salon"
            create_room_button.clicked.connect ( () => {
                    var dialog = new Bavardage.Widgets.NewRoomDialog (window, this);
                    dialog.create_new_room.connect ((room_name, is_sec) => {
                            string error_msg;
                            TextIter iter;
                            chat.get_buffer ().get_end_iter (out iter);
                            if (is_sec) {
                                secure_rooms.add (room_name);
                                if (send_message_sec ("/CREATE_ROOM_SEC " + room_name, out error_msg) == -3) {
                                    chat.get_buffer ().insert_text (ref iter, error_msg, error_msg.length);
                                }
                            } else {
                                if (send_message ("/CREATE_ROOM " + room_name, out error_msg) == -3) {
                                    chat.get_buffer ().insert_text (ref iter, error_msg, error_msg.length);
                                }
                            }


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
                            string error_msg;
                            if (!is_secure_room (s)) {
                                if (send_message ("/QUIT_ROOM " + s, out error_msg) == -3) {
                                    TextIter titer;
                                    chat.get_buffer ().get_end_iter (out titer);
                                    chat.get_buffer ().insert_text (ref titer, error_msg, error_msg.length);
                                }
                            } else {
                                if (send_message_sec ("/QUIT_ROOM_SEC " + s, out error_msg) == -3) {
                                    TextIter titer;
                                    chat.get_buffer ().get_end_iter (out titer);
                                    chat.get_buffer ().insert_text (ref titer, error_msg, error_msg.length);
                                }
                            }
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

                            model2.get_iter_first (out iter);
                            open_rooms.get_selection ().select_iter (iter);
                            open_rooms.cursor_changed ();
                        }
                    }
                });

            join_room_button.clicked.connect ( () => {
                    var dialog = new Bavardage.Widgets.JoinRoomDialog (window, this);
                    dialog.join_room.connect ((room_name, is_secure) => {
                            // demander à rejoindre le salon
                            string error_msg;

                            if (is_secure) {
                                secure_rooms.add (room_name);
                                if (send_message_sec ("/JOIN_ROOM_SEC " + room_name, out error_msg) == -3) {
                                    TextIter iter;
                                    chat.get_buffer ().get_end_iter (out iter);
                                    chat.get_buffer ().insert_text (ref iter, error_msg, error_msg.length);
                                }
                            } else {
                                if (send_message ("/JOIN_ROOM " + room_name, out error_msg) == -3) {
                                    TextIter iter;
                                    chat.get_buffer ().get_end_iter (out iter);
                                    chat.get_buffer ().insert_text (ref iter, error_msg, error_msg.length);
                                }
                            }
                        });
                    dialog.show_all ();
                });

            send_mp_button.clicked.connect ( () => {
                    TreeModel m;
                    TreeIter iter;
                    bool need_security = false;
                    var select = open_rooms.get_selection ();
                    if (select.get_selected (out m, out iter)) {
                        Value v;
                        m.get_value (iter, 0, out v);
                        if (is_secured) {
                            Gtk.MessageDialog msg = new Gtk.MessageDialog (window, Gtk.DialogFlags.MODAL, Gtk.MessageType.QUESTION, Gtk.ButtonsType.YES_NO, "Voulez-vous une communication chiffrée avec cet utilisateur ?");
                            msg.response.connect ( (response_id) => {
                                switch (response_id) {
                                case ResponseType.YES:
                                    need_security = true;
                                    break;
                                default:
                                    break;
                                }
                                msg.hide_on_delete ();
                                select = connected_users.get_selection ();
                                if (select.get_selected (out m, out iter)) {
                                    m.get_value (iter, 0, out v);
                                    string s = (string) v;
                                    string room_name = "[" + s + "]";
                                    if (rooms_map_chats.get (room_name) == null ) {
                                        var rooms = open_rooms.get_model () as ListStore;
                                     
                                        rooms.append (out iter);
                                        if (need_security) {
                                            secure_rooms.add (room_name);
                                            var img = new Gtk.Image.from_file ("channel-secure-symbolic.svg").get_pixbuf ();
                                            rooms.set (iter, 0, room_name, 1, img, -1);
                                        } else {
                                            rooms.set (iter, 0, room_name, 1, null, -1);
                                        }

                                        rooms_map_users.set (room_name, new ListStore (2, typeof (string), typeof (string)));
                                        var buffer = new TextBuffer (new TextTagTable ());
                                        buffer.create_tag ("blue", "foreground", "#0000FF", "underline", Pango.Underline.SINGLE);
                                        rooms_map_chats.set (room_name, buffer);
                                        rooms_map_entries.set (room_name, new EntryBuffer ("".data));
                                        open_rooms.get_selection ().select_iter (iter);
                                        open_rooms.cursor_changed ();
                                    }
                                }
                            });
                            msg.present ();
                        } else {
                            select = connected_users.get_selection ();
                            if (select.get_selected (out m, out iter)) {
                                m.get_value (iter, 0, out v);
                                string s = (string) v;
                                string room_name = "[" + s + "]";
                                if (rooms_map_chats.get (room_name) == null ) {
                                    var rooms = open_rooms.get_model () as ListStore;
                                 
                                    rooms.append (out iter);
                                    rooms.set (iter, 0, room_name, 1, null, -1);

                                    rooms_map_users.set (room_name, new ListStore (2, typeof (string), typeof (string)));
                                    var buffer = new TextBuffer (new TextTagTable ());
                                    buffer.create_tag ("blue", "foreground", "#0000FF", "underline", Pango.Underline.SINGLE);
                                    rooms_map_chats.set (room_name, buffer);
                                    rooms_map_entries.set (room_name, new EntryBuffer ("".data));
                                    open_rooms.get_selection ().select_iter (iter);
                                    open_rooms.cursor_changed ();
                                }
                            }
                        }
                    }
                    
                });

            invite_button.clicked.connect ( () => {
                    var dialog = new Bavardage.Widgets.SendInvitationDialog (window, this);
                    dialog.response.connect ( (responseid) => {
                            if (responseid == Gtk.ResponseType.ACCEPT) {
                                TreeModel m;
                                TreeIter iter;
                                var select = open_rooms.get_selection ();
                                if (select.get_selected (out m, out iter)) {
                                    Value v;
                                    m.get_value (iter, 0, out v);
                                    send_message_sec ("/ACCEPT_JOIN_ROOM_SEC " + (string) v +  " " + dialog.entry_login.get_text (), null);
                                }
                            }
                            dialog.hide_on_delete ();
                            });
                        dialog.show ();
                        });

                kick_button.clicked.connect ( () => {
                    TreeModel m;
                    TreeIter iter;
                    var select = connected_users.get_selection ();
                    if (select.get_selected (out m, out iter)) {
                        Value v;
                        m.get_value (iter, 0, out v);
                        string user_to_kick = (string) v;
                        select = open_rooms.get_selection ();
                        if (select.get_selected (out m, out iter)) {
                             m.get_value (iter, 0, out v);
                             string room = (string) v;
                            send_message_sec ("/EJECT_FROM_ROOM_SEC " + room + " " + user_to_kick, null);
                        }
                    }
                });

                message.activate.connect ( () => {
                        send_message_entry.begin ();
                    });

                // On clique sur le bouton "Envoyer"
                message.icon_press.connect ( (p0, p1) => {
                        send_message_entry.begin ();
                    });


                // On clique sur Fichier > Connexion
                connect_item.activate.connect ( () => {
                        conn_dial.present ();
                    });

                // On clique sur Fichier > Déconnexion
                disconnect_item.activate.connect ( () => {
                        // démander déconnexion
                        ClientCore.disconnect ();
                        update_connected (false, "");
                    });

                delete_sec_account_item.activate.connect ( () => {
                    var dialog = new Gtk.MessageDialog (window, Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT, Gtk.MessageType.WARNING, Gtk.ButtonsType.YES_NO, "Êtes-sûr de vouloir supprimer votre compte ?");
                    dialog.response.connect ( (response_id) => {
                        if (response_id == Gtk.ResponseType.YES) {
                            send_message_sec ("/DEL_ACCOUNT_SEC", null);
                        }
                        dialog.hide_on_delete ();
                    });
                    dialog.present ();
                });
                
                // On clique sur Fichier > À propos
                about_item.activate.connect ( () => {
                        Gtk.AboutDialog dialog = new Bavardage.Widgets.AboutDialog (window);

                        dialog.response.connect ((response_id) => {
                                if (response_id == Gtk.ResponseType.CANCEL || response_id == Gtk.ResponseType.DELETE_EVENT) {
                                    dialog.hide_on_delete ();
                                }
                            });
                        dialog.present ();
                    });

                // Écoute les changements d'état connecté/déconnecté
                conn_dial.update_connected.connect ( (is_connected, login, is_secure) => {
                        update_statusbar (is_connected, login);
                        if (is_secure) {
                            secure_statusbar.push (statusbar.get_context_id ("securestatus"), "(Connexion sécurisée)");
                            delete_sec_account_item.set_sensitive (true);
                        }
                        
                    });
                this.update_connected.connect ( (is_connected, login) => {
                        update_statusbar (is_connected, login);
                    });

                chat.button_release_event.connect ( (e) => {
                        TextIter iter;
                        chat.get_iter_at_location (out iter, (int) e.x, (int) e.y);
                        TextIter start;
                        chat.get_buffer ().get_start_iter (out start);

                        string s = chat.get_buffer ().get_text (start, iter, true);

                        TextIter end;
                        chat.get_buffer ().get_end_iter (out end);
                        string buffer_text = chat.get_buffer ().get_text (start, end, true);

                        int idx = s.last_index_of ("://");
                        if (idx == -1) {
                            iter.forward_chars (6);
                            s = chat.get_buffer ().get_text (start, iter, true);
                            idx = s.last_index_of ("://");
                        }
                        if (idx != -1 && !s.substring (idx, -1).contains (" ")) {
                            while (buffer_text[idx] != ' ' && idx > 0) {
                                idx--;
                            }
                            idx++;
                            StringBuilder url = new StringBuilder ("");
                            for (int i = idx; i < buffer_text.length; i++) {
                                if (buffer_text[i] == ' ' || buffer_text[i] == '\n') {
                                    break;
                                } else {
                                    url.append_c ((char) buffer_text[i]);
                                }
                            }
                            try {
                                Gtk.show_uri (null, url.str, Gdk.CURRENT_TIME);
                            } catch (GLib.Error e) {
                                stderr.printf (e.message);
                            }
                        }
                        return false;
                    });

                }

                private void update_statusbar (bool is_connected, string login) {
                if (!is_connected) {
                    statusbar.pop (statusbar.get_context_id ("status"));
                    statusbar.push (statusbar.get_context_id ("status"), "Déconnecté");
                } else {
                    statusbar.pop (statusbar.get_context_id ("status"));
                    statusbar.push (statusbar.get_context_id ("status"), "Connecté avec le login : " + login);

                }
                connect_item.set_sensitive (!is_connected);
                disconnect_item.set_sensitive (is_connected);
                create_room_button.set_sensitive (is_connected);
                quit_room_button.set_sensitive (is_connected);
                join_room_button.set_sensitive (is_connected);
                message.set_sensitive (is_connected);
                send_mp_button.set_sensitive (is_connected);
                invite_button.set_sensitive (is_connected);
                delete_sec_account_item.set_sensitive (false);
                kick_button.set_sensitive (false);
            }

            private async void send_message_entry () {
                var msg = message.get_text ();
                string error_msg;
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
                            if (is_secure_room ((string) v)) {
                                send_message_sec ("/MP_SEC " + recv_name.str + " " + msg, out error_msg);
                            } else {
                                send_message ("/MP " + recv_name.str + " " + msg, out error_msg);
                            }
                        } else {
                            if (is_secure_room ((string) v)) {
                                send_message_sec ("/MESSAGE " + (string) v + " " + msg, out error_msg);
                            } else {
                                send_message ("/MESSAGE " + (string) v + " " + msg, out error_msg);
                            }
                        }

                    }
                } else {
                    if (send_message (msg,out error_msg) == -3) {
                        TextIter titer;
                        chat.get_buffer ().get_end_iter (out titer);
                        chat.get_buffer ().insert_text (ref titer, error_msg, error_msg.length);
                    }
                }
                message.set_text("");
            }

            public bool is_secure_room (string room_name) {
                return secure_rooms.contains (room_name);
            }
        }

        /*
         * Point d'entrée du programme
         */
        int main (string[] args) {
            Gdk.init (ref args);
            Gtk.init (ref args);
            Bavardage.Common.init_rooms ();
            new Bavardage.Client (args[0]);
            Gtk.main ();
            return 0;
        }

    }

