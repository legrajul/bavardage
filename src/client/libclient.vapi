namespace Bavardage {
[CCode (cheader_filename = "../common/common.h")]
namespace Common {
    [CCode (cname = "message")]
    public struct Message {
        int code;
        string sender;
        string content;
        string receiver;
    }
}

[CCode (cprefix = "", lower_case_cprefix = "", cheader_filename = "libclient.h")]
namespace ClientCore {
    public int send_message (string m);
    public int send_command ();
    public int connect_socket (string addr, int port);
    public int disconnect ();
    public int get_last_request_code ();
    public bool is_connected ();
    public int receive_message (out Common.Message m);
}
}