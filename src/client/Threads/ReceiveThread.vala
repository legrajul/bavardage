using Bavardage.Common;
using Bavardage.CommonSec;
using Bavardage.ClientCore;
using Bavardage.ClientSecCore;
using Gtk;
using Gee;

namespace Bavardage.Threads {
    public class ReceiveThread {
        private string name;
        private Bavardage.Client client;
        private Gdk.Pixbuf img;

        public ReceiveThread (string name, Bavardage.Client client) {
            this.name = name;
            this.client = client;
        }

        public void  *thread_func () {
            img = Gtk.IconTheme.get_default ().load_icon ("channel-secure-symbolic", 16, 0);
            Bavardage.Message m = { -1, "".data, "".data, "".data };
            TreeIter tree_iter;
            while (true) {
                Thread.usleep (10000);
                m = { -1, "".data, "".data, "".data };
                if (receive_message (out m) == 0) {
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
                    case CREATE_ROOM_KO:
                    case JOIN_ROOM_KO:
                    case DELETE_ROOM_KO:
                    case MESSAGE_KO:
                    case MP_KO:
                    case QUIT_ROOM_KO:
                    case KO:
                        string s = "Erreur : " + content.str + "\n";
                        TextIter iter;
                        client.chat.get_buffer ().get_end_iter (out iter);
                        client.chat.get_buffer ().insert_text (ref iter, s, s.length);

                        break;
                    case OK:

                        break;
                    case CREATE_ROOM:
                        var rooms = client.open_rooms.get_model () as ListStore;
                        rooms.append (out tree_iter);
                        if (client.is_secure_room (content.str)) {
                            rooms.set (tree_iter, 0, content.str, 1, img, -1);
                        } else {
                            rooms.set (tree_iter, 0, content.str, 1, null, -1);
                        }
                        client.rooms_map_users.set (content.str, new ListStore (2, typeof (string), typeof (string)));
                        var buffer = new TextBuffer (new TextTagTable ());
                        buffer.create_tag ("blue", "foreground", "#0000FF", "underline", Pango.Underline.SINGLE);
                        client.rooms_map_chats.set (content.str, buffer);
                        client.rooms_map_entries.set (content.str, new EntryBuffer ("".data));
                        client.open_rooms.get_selection ().select_iter (tree_iter);
                        client.open_rooms.cursor_changed ();
                        break;
                    case DELETE_ROOM:
                        Value v;
                        var model = client.open_rooms.get_model () as ListStore;
                        model.get_iter_first (out tree_iter);
                        do {
                            model.get_value (tree_iter, 0, out v);
                            if (content.str == (string) v) {
                                break;
                            }
                        } while (model.iter_next (ref tree_iter));
                        model.remove (tree_iter);
                        client.rooms_map_chats.unset (content.str);
                        client.rooms_map_entries.unset (content.str);
                        client.rooms_map_users.unset (content.str);
                        model.get_iter_first (out tree_iter);
                        client.open_rooms.get_selection ().select_iter (tree_iter);
                        client.open_rooms.cursor_changed ();
                        break;
                    case MESSAGE:
                        string s = "<" + sender.str + "> ";
                        if (client.is_secure_room (receiver.str)) {
                            var tmptext = decrypt (receiver.str, m.content);
                            if (tmptext.validate ()) {
                                s += tmptext;
                            } else if (content.str.validate ()) {
                                s += content.str;
                            } else {
                                s += "*** MESSAGE ILLISIBLE ***";
                            }
                        } else {
                            if (content.str.validate ()) {
                                s += content.str;
                            } else {
                                s += "*** MESSAGE ILLISIBLE ***";
                            }
                        }
                        s += "\n";
                        TextIter iter;
                        client.rooms_map_chats.get (receiver.str).get_end_iter (out iter);
                        client.rooms_map_chats.get (receiver.str).insert_text (ref iter, s, s.length);

                        TextIter start, end;
                        client.rooms_map_chats.get (receiver.str).get_end_iter (out end);
                        client.rooms_map_chats.get (receiver.str).get_start_iter (out start);
                        string buffer_text = client.rooms_map_chats.get (receiver.str).get_text (start, end, true);
                        if (buffer_text.contains ("://")) {
                            int idx = buffer_text.index_of ("://");

                            int old_idx = 0;
                            while (idx != -1) {
                                while (idx > 0 && buffer_text[idx] != ' ') {
                                    idx--;
                                }
                                idx++;
                                StringBuilder url = new StringBuilder ("");
                                int i = 0;
                                for (i = idx; i < buffer_text.length; i++) {
                                    if (buffer_text[i] == ' ' || buffer_text[i] == '\n') {
                                        break;
                                    } else {
                                        url.append_c ((char) buffer_text[i]);
                                    }
                                }

                                client.rooms_map_chats.get (receiver.str).get_iter_at_offset (out start, idx);
                                client.rooms_map_chats.get (receiver.str).get_iter_at_offset (out end, idx + url.str.length);
                                client.rooms_map_chats.get (receiver.str).apply_tag_by_name ("blue", start, end);

                                old_idx = idx;
                                idx = buffer_text.index_of ("://", i + 1);
                            }
                        }
                        if (client.chat.get_buffer () == client.rooms_map_chats.get (receiver.str)) {
                            client.chat.get_buffer ().get_iter_at_line (out iter, client.chat.get_buffer ().get_line_count ());
                            client.chat.scroll_to_iter (iter, 0.0, false, 0.0, 1.0);
                            client.chat.get_buffer ().set_modified (true);
                        }
                        break;

                    case MP:
                        string s = "[" + sender.str + "] ";
                        TextIter iter;
                        string room_name = "[" + sender.str + "]";
                        if (sender.str == ClientCore.get_login ()) {
                            room_name = "[" + receiver.str + "]";
                        }
                        if (client.rooms_map_chats.get (room_name) == null) {
                            var rooms = client.open_rooms.get_model () as ListStore;
                            rooms.append (out tree_iter);
                            rooms.set (tree_iter, 0, room_name, -1);

                            client.rooms_map_users.set (room_name, new ListStore (2, typeof (string), typeof (string)));
                            var buffer = new TextBuffer (new TextTagTable ());
                            buffer.create_tag ("blue", "foreground", "#0000FF", "underline", Pango.Underline.SINGLE);
                            client.rooms_map_chats.set (room_name, buffer);
                            client.rooms_map_entries.set (room_name, new EntryBuffer ("".data));
                            client.open_rooms.get_selection ().select_iter (tree_iter);
                            client.open_rooms.cursor_changed ();
                        }
                        client.rooms_map_chats.get (room_name).get_end_iter (out iter);

                        if (client.is_secure_room (room_name)) {
                            string text = "";
                            if (sender.str == ClientCore.get_login ()) {
                                text += decrypt (receiver.str, m.content);
                            } else {
                                text += decrypt (sender.str, m.content);
                            }
                            if (text.validate ()) {
                                s += text + "\n";
                            } else {
                                s += "*** MESSAGE ILLISIBLE ***\n";
                            }
                        } else {
                            if (content.str.validate ()) {
                                s += content.str + "\n";
                            } else {
                                s += "*** MESSAGE ILLISIBLE ***\n";
                            }
                        }
                        
                        client.rooms_map_chats.get (room_name).insert_text (ref iter, s, s.length);

                        TextIter start, end;
                        client.rooms_map_chats.get (room_name).get_start_iter (out start);
                        client.rooms_map_chats.get (room_name).get_end_iter (out end);
                        string buffer_text = client.rooms_map_chats.get (room_name).get_text (start, end, true);
                        if (buffer_text.contains ("://")) {
                            int idx = buffer_text.index_of ("://");

                            int old_idx = 0;
                            while (idx != -1) {
                                while (idx > 0 && buffer_text[idx] != ' ') {
                                    idx--;
                                }
                                idx++;
                                int i = 0;
                                StringBuilder url = new StringBuilder ("");
                                for (i = idx; i < buffer_text.length; i++) {
                                    if (buffer_text[i] == ' ' || buffer_text[i] == '\n') {
                                        break;
                                    } else {
                                        url.append_c ((char) buffer_text[i]);
                                    }
                                }

                                client.rooms_map_chats.get (room_name).get_iter_at_offset (out start, idx);
                                client.rooms_map_chats.get (room_name).get_iter_at_offset (out end, idx + url.str.length);
                                client.rooms_map_chats.get (room_name).apply_tag_by_name ("blue", start, end);

                                old_idx = idx;
                                idx = buffer_text.index_of ("://", i + 1);
                            }
                        }

                        if (client.chat.get_buffer () == client.rooms_map_chats.get (receiver.str)) {
                            client.chat.get_buffer ().get_iter_at_line (out iter, client.chat.get_buffer ().get_line_count ());
                            client.chat.scroll_to_iter (iter, 0.0, false, 0.0, 1.0);
                            client.chat.get_buffer ().set_modified (true);
                        }
                        break;
                    case NEW_USER:
                        string s = sender.str + " vient de rejoindre le salon\n";
                        TextIter iter;
                        string room_name = content.str;
                        client.rooms_map_chats.get (room_name).get_end_iter (out iter);
                        client.rooms_map_chats.get (room_name).insert_text (ref iter, s, s.length);
                        var users = client.rooms_map_users.get (content.str) as ListStore;
                        users.append (out tree_iter);
                        users.set (tree_iter, 0, sender.str, 1, "", -1);
                        if (client.chat.get_buffer () == client.rooms_map_chats.get (receiver.str)) {
                            client.chat.get_buffer ().get_iter_at_line (out iter, client.chat.get_buffer ().get_line_count ());
                            client.chat.scroll_to_iter (iter, 0.0, false, 0.0, 1.0);
                            client.chat.get_buffer ().set_modified (true);
                        }
                        break;
                    case ADD_USER:
                        var users = client.rooms_map_users.get (content.str) as ListStore;
                        users.append (out tree_iter);
                        users.set (tree_iter, 0, sender.str, 1, "", -1);
                        break;
                    case RM_USER:
                        string s = sender.str + " vient de quitter le salon\n";
                        TextIter iter;
                        string room_name = content.str;
                        client.rooms_map_chats.get (room_name).get_end_iter (out iter);
                        client.rooms_map_chats.get (room_name).insert_text (ref iter, s, s.length);
                        Value v;
                        var model = client.rooms_map_users.get (content.str)as ListStore;
                        model.get_iter_first (out tree_iter);
                        do {
                            model.get_value (tree_iter, 0, out v);
                            if (sender.str == (string) v) {
                                break;
                            }
                        } while (model.iter_next (ref tree_iter));
                        model.remove (tree_iter);
                        if (client.chat.get_buffer () == client.rooms_map_chats.get (receiver.str)) {
                            client.chat.get_buffer ().get_iter_at_line (out iter, client.chat.get_buffer ().get_line_count ());
                            client.chat.scroll_to_iter (iter, 0.0, false, 0.0, 1.0);
                            client.chat.get_buffer ().set_modified (true);
                        }
                        break;
                    case DISCONNECT:
                        client.rooms_map_chats.clear ();
                        client.rooms_map_entries.clear ();
                        client.rooms_map_users.clear ();
                        client.open_rooms.set_model (new ListStore (2, typeof (string), typeof (Gdk.Pixbuf)));
                        client.chat.set_buffer (new TextBuffer (new TextTagTable ()));
                        client.message.set_buffer (new EntryBuffer ("".data));
                        client.connected_users.set_model (new ListStore (2, typeof (string), typeof (string)));
                        client.update_connected (false, "");
                        Thread.exit (null);
                        break;
                    case ADMIN:
                        var users = client.rooms_map_users.get (content.str) as ListStore;
                        users.append (out tree_iter);
                        users.set (tree_iter, 0, sender.str, 1, "(admin)", -1);
                        break;
                    default:
                        break;
                    }
                } else {
                    stdout.printf ("Error\n");
                    break;
                }
            }
            return null;
        }
    }
}
