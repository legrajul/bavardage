using Gtk;
using Bavardage.ClientCore;
using Bavardage.ClientSecCore;
using Bavardage.Common;
using Bavardage.CommonSec;

namespace Bavardage.Widgets {

    public class ConnectionDialog: Gtk.Dialog {
        public Entry entry_server_ip { get; set; }
        public Entry entry_server_port { get; set; }
        public Entry entry_login { get; set; }
        public Entry entry_password { get; set; }

        public Entry entry_serversec_ip { get; set; }
        public Entry entry_serversec_port { get; set; }

        private FileChooserButton cert_file_chooser_button;
        private FileChooserButton key_file_chooser_button;

        public File certif_file { get; set; }
        public File private_key_file { get; set; }

        private Grid connection_grid;
        private Grid secure_connection_grid;

        private CheckButton secure_connection_checkbox;

        public signal void connect_regular (string server_ip, int server_port, string login);

        public signal void connect_secure (string server_ip, int server_port, string login, string serversec_ip, int serversec_port, File cert_file, File private_key_file, string password);

        public signal void update_connected (bool is_connected, string login, bool is_secure);

        public ConnectionDialog (Gtk.Window parent, Gtk.Application app) {
            this.add_buttons (Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.CONNECT, Gtk.ResponseType.ACCEPT);
            this.set_title ("Connexion");
            this.set_modal (true);
            this.set_transient_for (parent);
            this.set_position (WindowPosition.CENTER_ON_PARENT);
            this.set_application (app);
            setup_view ();
            this.show_all ();
            secure_connection_grid.set_sensitive (false);
            this.check_resize ();
            connect_signals ();
        }


        private void setup_view () {
            connection_grid = new Grid (); {
                entry_server_ip = new Entry ();
                entry_server_port = new Entry ();
                entry_login = new Entry ();
                secure_connection_checkbox = new CheckButton.with_label ("Utiliser une connexion sécurisée");
            }

            secure_connection_grid = new Grid (); {
                entry_password = new Entry ();
                entry_password.set_visibility (false);
                entry_serversec_ip = new Entry ();
                entry_serversec_port = new Entry ();
                cert_file_chooser_button = new FileChooserButton ("Certificat", FileChooserAction.OPEN);
                FileFilter filter = new FileFilter ();
                cert_file_chooser_button.add_filter (filter);
                filter.add_mime_type ("application/x-x509-ca-cert");

                key_file_chooser_button = new FileChooserButton ("Clef privée", FileChooserAction.OPEN);
                key_file_chooser_button.add_filter (filter);

                Bavardage.Client cl = this.get_application () as Bavardage.Client;
                cert_file_chooser_button.set_current_folder (cl.exec_directory);
                key_file_chooser_button.set_current_folder (cl.exec_directory);
            }

            /* TEST SECTION */
            entry_server_ip.set_text ("servers");
            entry_server_port.set_text ("10000");
            entry_login.set_text ("");
            entry_serversec_ip.set_text ("servers");
            entry_serversec_port.set_text ("11000");
            entry_password.set_text ("");
            /* END TEST SECTION */

            connection_grid.attach (new Label ("Adresse du serveur :"), 0, 0, 1, 1);
            connection_grid.attach (entry_server_ip, 1, 0, 1, 1);
            connection_grid.attach (new Label ("Port du server :"), 0, 1, 1, 1);
            connection_grid.attach (entry_server_port, 1, 1, 1, 1);
            connection_grid.attach (new Label ("Login :"), 0, 2, 1, 1);
            connection_grid.attach (entry_login, 1, 2, 1, 1);
            connection_grid.attach (secure_connection_checkbox, 0, 3, 2, 2);


            secure_connection_grid.attach (new Label ("Adresse du serveur sécurisé :"), 0, 0, 1, 1);
            secure_connection_grid.attach (entry_serversec_ip, 1, 0, 1, 1);
            secure_connection_grid.attach (new Label ("Port du serveur sécurisé :"), 0, 1, 1, 1);
            secure_connection_grid.attach (entry_serversec_port, 1, 1, 1, 1);
            secure_connection_grid.attach (new Label ("Certificat :"), 0, 2, 1, 1);
            secure_connection_grid.attach (cert_file_chooser_button, 1, 2, 1, 1);
            secure_connection_grid.attach (new Label ("Clef privée :"), 0, 3, 1, 1);
            secure_connection_grid.attach (key_file_chooser_button, 1, 3, 1, 1);
            secure_connection_grid.attach (new Label ("Mot de passe :"), 0, 4, 1, 1);
            secure_connection_grid.attach (entry_password, 1, 4, 1, 1);

            var box = this.get_content_area ();
            box.set_orientation (Orientation.VERTICAL);
            box.pack_start (connection_grid, true, true);
            box.pack_start (secure_connection_grid, true, true);
        }

        private void connect_signals () {
            secure_connection_checkbox.toggled.connect ( () => {
                    secure_connection_grid.set_sensitive (secure_connection_checkbox.get_active ());
                });

            entry_server_ip.activate.connect ( () => {this.response (Gtk.ResponseType.ACCEPT);});
            entry_server_port.activate.connect ( () => {this.response (Gtk.ResponseType.ACCEPT);});
            entry_login.activate.connect ( () => {this.response (Gtk.ResponseType.ACCEPT);});
            entry_password.activate.connect ( () => {this.response (Gtk.ResponseType.ACCEPT);});
            entry_serversec_ip.activate.connect ( () => {this.response (Gtk.ResponseType.ACCEPT);});
            entry_serversec_port.activate.connect ( () => {this.response (Gtk.ResponseType.ACCEPT);});
            this.response.connect ( (response_code) => {
                    if (response_code == Gtk.ResponseType.ACCEPT) {
                        if (secure_connection_checkbox.get_active ()) {
                            this.connect_secure (entry_server_ip.get_text (), int.parse (entry_server_port.get_text ()), entry_login.get_text (), entry_serversec_ip.get_text (), int.parse (entry_serversec_port.get_text ()), cert_file_chooser_button.get_file (), key_file_chooser_button.get_file (), entry_password.get_text ());
                        } else {
                            this.connect_regular (entry_server_ip.get_text (), int.parse (entry_server_port.get_text ()), entry_login.get_text ());
                        }
                    } else {
                        this.hide_on_delete ();
                    }
                });

            this.connect_regular.connect ((chatip, chatport, login) => {
                    ClientCore.disconnect ();
                    if (connect_socket (entry_server_ip.get_text (), int.parse (entry_server_port.get_text ())) == -1) {
                        var msg = new MessageDialog (this, DialogFlags.MODAL | DialogFlags.DESTROY_WITH_PARENT, MessageType.ERROR, ButtonsType.OK, "Le serveur indiqué ne répond pas");
                        msg.response.connect ((response_id3) => {
                                msg.hide_on_delete ();
                            });
                        msg.present ();
                    } else {
                        string tmp;
                        if (send_message ("/CONNECT " + entry_login.get_text (), out tmp) == -1) {
                            var msg = new MessageDialog (this, DialogFlags.MODAL | DialogFlags.DESTROY_WITH_PARENT, MessageType.ERROR, ButtonsType.OK, "Login vide");
                            msg.response.connect ((response_id3) => {
                                    msg.hide_on_delete ();
                                });
                            msg.present ();
                        } else {
                            Message m;
                            receive_message (out m);
                            if (m.code == KO || m.code == CONNECT_KO) {
                                var content_str = new StringBuilder ("");
                                for (int i = 0; i < m.content.length; i++) {
                                    content_str.append_c ((char) m.content[i]);
                                }
                                var msg = new MessageDialog (this, DialogFlags.MODAL | DialogFlags.DESTROY_WITH_PARENT, MessageType.ERROR, ButtonsType.OK, content_str.str);
                                msg.response.connect ((response_id2) => {
                                        msg.hide_on_delete ();
                                        this.present ();
                                    });
                                msg.present ();
                            } else {
                                update_connected (true, entry_login.get_text (),false);
                                this.hide ();
                                try {
                                    var threadrcv = new Bavardage.Threads.ReceiveThread ("receive thread", this.application as Bavardage.Client);
                                    (application as Bavardage.Client).thread_receive = new Thread<void *>.try ("recv thread", threadrcv.thread_func);
                                } catch (GLib.Error e) {
                                    stderr.printf ("Error : %s\n", e.message);
                                }
                            }
                        }
                    }
                });

            this.connect_secure.connect ((chatip, chatport, login, secip, secport, cert, key, password) => {
                    init_OpenSSL ();
                    //TODO: implémenter les fonctions pour la partie sécurisée avec ClientCoreSec.vapi
                    set_certif_filename (cert.get_path ());
                    set_private_key_filename (key.get_path ());
                    set_private_key_password (password);
                    connect_with_authentication (chatip, chatport, secip, secport);

                    string error;
                    Message m;
                    int ret = send_message_sec ("/CONNECT_SEC " + login, out error);
                    
                    ret = receive_message_sec (out m);
                    if (m.code == KO || m.code == CONNECT_KO || m.code == CONNECT_SEC_KO) {
                        var content_str = new StringBuilder ("");
                        for (int i = 0; i < m.content.length; i++) {
                            content_str.append_c ((char) m.content[i]);
                        }
                        var msg = new MessageDialog (this, DialogFlags.MODAL | DialogFlags.DESTROY_WITH_PARENT, MessageType.ERROR, ButtonsType.OK, content_str.str);
                        msg.response.connect ((response_id2) => {
                                msg.hide_on_delete ();
                                this.present ();
                            });
                        msg.present ();
                    } else {
                        this.connect_regular (chatip, chatport, login);
                        update_connected (true, entry_login.get_text (), true);
                        this.hide ();
                        try {
                            var threadrcvsec = new Bavardage.Threads.ReceiveSecThread ("receive thread", this.application as Bavardage.Client);
                            (application as Bavardage.Client).thread_receive_sec = new Thread<void *>.try ("recv thread sec", threadrcvsec.thread_func);
                        } catch (GLib.Error e) {
                            stderr.printf ("Error : %s\n", e.message);
                        }
                    }
                });
        }

        public bool is_secure_connect () {
            return secure_connection_checkbox.get_active ();
        }

    }
}
