struct DNS_HEADER
{
    unsigned    short id;            // identification number
    
    unsigned    char rd     :1;        // recursion desired
    unsigned    char tc     :1;        // truncated message
    unsigned    char aa     :1;        // authoritive answer
    unsigned    char opcode :4;        // purpose of message
    unsigned    char qr     :1;        // query/response flag
    
    unsigned    char rcode  :4;        // response code
    unsigned    char cd     :1;        // checking disabled
    unsigned    char ad     :1;        // authenticated data
    unsigned    char z      :1;        // its z! reserved
    unsigned    char ra     :1;        // recursion available
    
    unsigned    short q_count;       // number of question entries
    unsigned    short ans_count;     // number of answer entries
    unsigned    short auth_count;    // number of authority entries
    unsigned    short add_count;     // number of resource entries
};

struct  RR_DATA
{
    unsigned short type;   //two octets containing one of the RR TYPE codes
    unsigned short _class;
    unsigned int   ttl;
    unsigned short rdlen;    // length of rdata
};
#define DNS_OFFSET_MASK       0xC0 //1100
#define DNS_RR_DATA_HEAD_SIZE (3*sizeof(unsigned short)+ sizeof(unsigned int))

using namespace std;
struct DNS_QUESTION
{
    unsigned char* qname;
    unsigned short qtype;
    unsigned short qclass;
};

struct  DNS_RRS_DATA
{
    unsigned char* name;
    unsigned short type;
    unsigned short _class;
    unsigned int   ttl;
    unsigned short rdlen;
    unsigned char* r_data;
};

struct DNS_QUERY
{
    struct DNS_HEADER DnsHeader;
    struct DNS_QUESTION DnsQuestion;
    struct DNS_RRS_DATA *DnsAnswer;
    struct DNS_RRS_DATA *Authority;
    struct DNS_RRS_DATA *Additional;
};
