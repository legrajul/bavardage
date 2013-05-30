using Bavardage.Common;
using Bavardage.CommonSec;
using Bavardage.ClientCore;
using Bavardage.ClientSecCore;
using Gtk;
using Gee;

namespace Bavardage.Threads {
    public class ReceiveSecThread {
        private string name;
        private Bavardage.Client client;

        public ReceiveSecThread (string name, Bavardage.Client client) {
            this.name = name;
            this.client = client;
        }

        public void *thread_func () {
            var img = new Gtk.Image.from_file ("channel-secure-symbolic.svg").get_pixbuf ();
            Message m = { -1, "".data, "".data, "".data };
            while (true) {
                Thread.usleep (10000);
                m = { -1, "".data, "".data, "".data };
                if (receive_message_sec (out m) == 0) {
                    var sender = new StringBuilder ("");
                    for (int i = 0; i < m.sender.length; i++) {
                        sender.append_c((char) m.sender[i]);
                    }
                    var content = new StringBuilder ("");
                    for (int i = 0; i < m.content.length && m.content[i] != '\0'; i++) {
                        content.append_c ((char) m.content[i]);
                    }
                    var receiver = new StringBuilder ("");
                    for (int i = 0; i < m.receiver.length; i++) {
                        receiver.append_c ((char) m.receiver[i]);
                    }
                    switch (m.code) {
                    case KO:
                        string s = "Erreur : " + content.str + "\n";
                        TextIter iter;
                        client.chat.get_buffer ().get_end_iter (out iter);
                        client.chat.get_buffer ().insert_text (ref iter, s, s.length);

                        break;
                    case CONNECT_SEC:
                        client.is_secured = true;
                        client.secure_statusbar.push (client.statusbar.get_context_id ("securestatus"), "(Connexion sécurisée)");
                        break;
                    case CREATE_ROOM_SEC:
                        var room_name = new StringBuilder ("");
                        for (int i = 0; i < m.content.length && m.content[i] != '|'; i++) {
                            room_name.append_c ((char) m.content[i]);
                        }
                        if (!client.rooms_map_chats.has_key (room_name.str)) {
                            client.secure_rooms.add (room_name.str);
                            send_message ("/JOIN_ROOM " + room_name.str, null);
                        }
                        break;
                    case MP_SEC_OK:
                        
                        var room_name = new StringBuilder ("");
                        for (int i = 0; i < m.content.length && m.content[i] != '|'; i++) {
                            room_name.append_c ((char) m.content[i]);
                        }
                        client.secure_rooms.add ("[" + room_name.str + "]");
                        break;
                    case ASK_JOIN_ROOM_SEC:
                        string s = sender.str + " souhaite pouvoir échanger des messages chiffrés sur ce salon\n";
                        TextIter iter;
                        string room_name = content.str;
                        client.rooms_map_chats.get (room_name).get_end_iter (out iter);
                        client.rooms_map_chats.get (room_name).insert_text (ref iter, s, s.length);
                        if (client.chat.get_buffer () == client.rooms_map_chats.get (receiver.str)) {
                            client.chat.get_buffer ().get_iter_at_line (out iter, client.chat.get_buffer ().get_line_count ());
                            client.chat.scroll_to_iter (iter, 0.0, false, 0.0, 1.0);
                            client.chat.get_buffer ().set_modified (true);
                        }
                        break;
                    case QUIT_ROOM_SEC:
                        client.secure_rooms.remove (content.str);
                        break;
                    case DELETE_ROOM_SEC:
                        client.secure_rooms.remove (content.str);
                        break;
                    case DISCONNECT_SEC:
                        Thread.exit (null);
                        break;
                    case NEW_USER:
                        stdout.printf ("NEW_USER\n");
                        client.secure_users.add (sender.str);
                        TreeIter tree_iter;
                        Value v1, v2;
                        v1 = v2 = "";
                        var users = client.rooms_map_users.get (content.str) as ListStore;
                        users.get_iter_first (out tree_iter);
                        do {
                            users.get_value (tree_iter, 0, out v1);
                            users.get_value (tree_iter, 1, out v2);
                            if (sender.str == (string) v1) {
                                users.remove (tree_iter);
                                
                                break;
                            }
                        } while (users.iter_next (ref tree_iter));
                        users.set (tree_iter, 0, v1, 1, v2, 2, img, null);
                        break;
                    case ADD_USER:
                        stdout.printf ("ADD_USER\n");
                        client.secure_users.add (sender.str);
                        TreeIter tree_iter;
                        Value v1, v2;
                        v1 = v2 = "";
                        var users = client.rooms_map_users.get (content.str) as ListStore;
                        users.get_iter_first (out tree_iter);
                        do {
                            users.get_value (tree_iter, 0, out v1);
                            users.get_value (tree_iter, 1, out v2);
                            if (sender.str == (string) v1) {
                                users.remove (tree_iter);
                                
                                break;
                            }
                        } while (users.iter_next (ref tree_iter));
                        users.set (tree_iter, 0, v1, 1, v2, 2, img, null);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}
