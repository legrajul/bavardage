namespace Bavardage.Secure {
    [CCode (cprefix = "", lower_case_cprefix = "", cheader_filename = "../common/commonsec.h")]
    namespace Common {
        public const int CREATE_ROOM_SEC;
        public const int DELETE_ROOM_SEC;
        public const int QUIT_ROOM_SEC;
        public const int JOIN_ROOM_SEC;
        public const int DISCONNECT_SEC;
        public const int CONNECT_SEC;
    }

    
    [CCode (cprefix = "", lower_case_cprefix = "", cheader_filename = "libclientsec.h")]
    namespace ClientSecCore {
        public int connect_with_authentication (string chatservaddr, int chatservport, string login, string secservaddr, int secservport);

        public int disconnect_servers ();

        public void set_certif_filename (string filename);

        public void set_private_key_filename (string filename);
    }
}
