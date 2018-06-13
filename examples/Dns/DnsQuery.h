/*

MIT License

Copyright (c) 2018 Chris McArthur, prince.chrismc(at)gmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

struct DNS_HEADER
{
    unsigned    short id;            // identification number

    unsigned    char rd : 1;        // recursion desired
    unsigned    char tc : 1;        // truncated message
    unsigned    char aa : 1;        // authoritive answer
    unsigned    char opcode : 4;        // purpose of message
    unsigned    char qr : 1;        // query/response flag

    unsigned    char rcode : 4;        // response code
    unsigned    char cd : 1;        // checking disabled
    unsigned    char ad : 1;        // authenticated data
    unsigned    char z : 1;        // its z! reserved
    unsigned    char ra : 1;        // recursion available

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
