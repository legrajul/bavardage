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
            stdout.printf ("Receive Sec Thread starting...\n");
            Message m = { -1, "".data, "".data, "".data };
            TreeIter tree_iter;
            while (true) {
                Thread.usleep (10000);
                m = { -1, "".data, "".data, "".data };
                if (receive_message_sec (out m) == 0) {
                    stdout.printf ("MESSAGE SEC RECU, code = %d\n", m.code);
                    switch (m.code) {
                    case CONNECT_SEC:
                        client.is_secured = true;
                        client.secure_statusbar.push (client.statusbar.get_context_id ("securestatus"), "(Connexion sécurisée)");
                        break;
                    case CREATE_ROOM_SEC:
                        stdout.printf ("CREATE_ROOM_SEC received\n");
                        break;
                    default:
                        break;
                    }
                    stdout.printf ("Message reçu !\n");
                }
            }
            Thread.exit (null);
            return null;
        }
    }
}