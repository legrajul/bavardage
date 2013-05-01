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
            Message m = { -1, "".data, "".data, "".data };
            TreeIter tree_iter;
            while (true) {
                Thread.usleep (10000);
                m = { -1, "".data, "".data, "".data };
                if (receive_message_sec (out m) == 0) {
                    switch (m.code) {
                    case CONNECT_SEC:
                        client.is_secured = true;
                        client.secure_statusbar.push (client.statusbar.get_context_id ("securestatus"), "(Connexion sécurisée)");
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