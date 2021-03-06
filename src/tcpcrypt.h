#ifndef __SRC_TCPCRYPT_H__
#define __SRC_TCPCRYPT_H__

#include <tcpcrypt/tcpcrypt.h>
#include "tcpcrypt_ctl.h"
#include "tcpcrypt_version.h"

#define TC_DUMMY	0x69

#define TC_OPT_VLEN	0x80

enum {
	TC_RESUME	     = 0x20,
	TC_CIPHER_ECDHE_P256 = 0x21,
	TC_CIPHER_ECDHE_P521 = 0x22,
};

enum {
	TC_AES128_GCM = 0x01,
	TC_AES256_GCM = 0x02,
};

enum {
	TC_HMAC_SHA1_128	= 0x01,
	TC_UMAC,
};

enum {
	CONST_NEXTK	= 0x01,
	CONST_SESSID	= 0x02,
	CONST_REKEY	= 0x03,
	CONST_KEY_C	= 0x04,
	CONST_KEY_S	= 0x05,
	CONST_KEY_ENC	= 0x06,
	CONST_KEY_MAC	= 0x07,
	CONST_KEY_ACK	= 0x08,
};

struct tc_cipher_spec {
	uint8_t  tcs_algo;
} __attribute__ ((gcc_struct, __packed__));

struct tc_scipher {
	uint8_t sc_algo;
};

enum {
	STATE_RDR_NONE = 0,
	STATE_RDR_LOCAL,
	STATE_RDR_REMOTE,
};

enum {
	STATE_CLOSED		=  0,
	STATE_HELLO_SENT,
	STATE_HELLO_RCVD,
	STATE_PKCONF_SENT,
	STATE_PKCONF_RCVD,
	STATE_INIT1_SENT	=  5,
	STATE_INIT1_RCVD,
	STATE_INIT2_SENT,
	STATE_ENCRYPTING,
	STATE_DISABLED,
	STATE_NEXTK1_SENT	= 10,
	STATE_NEXTK1_RCVD,
	STATE_NEXTK2_SENT,
	STATE_REKEY_SENT,
	STATE_REKEY_RCVD,
	STATE_RDR_PLAIN		= 15,
};

enum {
	CMODE_DEFAULT	= 0,
	CMODE_ALWAYS,
	CMODE_ALWAYS_NK,
	CMODE_NEVER,
	CMODE_NEVER_NK,
};

enum {
	ROLE_CLIENT	= 1,
	ROLE_SERVER,
};

enum {
	TCPSTATE_CLOSED	= 0,
	TCPSTATE_FIN1_SENT,
	TCPSTATE_FIN1_RCVD,
	TCPSTATE_FIN2_SENT,
	TCPSTATE_FIN2_RCVD,
	TCPSTATE_LASTACK,
	TCPSTATE_DEAD,
};

struct crypt_alg {
	struct crypt_ops	*ca_ops;
	void			*ca_priv;
};

#define MAX_SS		32

struct stuff {
	uint8_t	s_data[MAX_SS * 2];
	int	s_len;
};

struct tc_sess {
	struct crypt_pub	*ts_pub;
	struct crypt_sym	*ts_sym;
	struct crypt_alg	ts_mac;
	struct stuff		ts_sid;
	struct stuff		ts_nk;
	struct stuff		ts_mk;
	uint8_t			ts_pub_spec;
	int			ts_role;
	struct in_addr		ts_ip;
	int			ts_port;
	int			ts_dir;
	struct tc_sess		*ts_next;
	int			ts_used;
};

struct tc_sid {
        uint8_t ts_sid[9];
} __attribute__ ((__packed__));

struct tc_sess_opt {
	uint8_t	      ts_opt;
	struct tc_sid ts_sid;
} __attribute__ ((__packed__));

#define TCF_FIN 0x1
#define TCF_URG 0x2

struct tc_flags {
	uint8_t		tf_flags;
	uint16_t	tf_urp[0];
} __attribute__ ((__packed__));

#define TC_MTU		1500
#define MAX_CIPHERS	8
#define MAX_NONCE	48

enum {
	IVMODE_NONE	= 0,
	IVMODE_SEQ,
	IVMODE_CRYPT,
};

enum {
	DIR_IN	= 1,
	DIR_OUT,
};

struct tc_keys {
	struct stuff	tk_prk;
};

struct tc_keyset {
	struct tc_keys		tc_client;
	struct tc_keys		tc_server;
	struct crypt_sym	*tc_alg_tx;
	struct crypt_sym	*tc_alg_rx;
};

/* Contains vanilla sequence numbers as received by tcpcryptd.  off is the
 * padding (tc_record) added (or removed) by tcpcryptd to that packet.
 *
 * kernel   -> tcpcryptd.  [add off]
 * internet -> tcpcryptd.  [sub off]
 */
struct tc_seq {
	uint32_t sm_start;
	uint32_t sm_end;
	uint32_t sm_off;
};

/* should be proportional to window size (in packets) */
#define MAX_SEQMAP	100

struct tc_seqmap {
	struct tc_seq sm_seq[MAX_SEQMAP];
	int	      sm_idx;
};

struct conn;

struct tc {
	int			tc_state;
	struct tc_cipher_spec	*tc_ciphers_pkey;
	int			tc_ciphers_pkey_len;
	struct tc_scipher	*tc_ciphers_sym;
	int			tc_ciphers_sym_len;
	struct tc_cipher_spec	tc_cipher_pkey;
	struct tc_scipher	tc_cipher_sym;
	struct crypt_pub	*tc_crypt_pub;
	struct crypt_sym	*tc_crypt_sym;
	int			tc_mac_size;
	int			tc_mac_ivlen;
	int			tc_mac_ivmode;
	uint64_t		tc_seq;
	uint64_t		tc_ack;
	void			*tc_crypt;
	struct crypt_ops	*tc_crypt_ops;
	int			tc_mac_rst;
	int			tc_cmode;
	int			tc_tcp_state;
	int			tc_mtu;
	struct tc_sess		*tc_sess;
	int			tc_mss_clamp;
	int			tc_seq_off;
	int			tc_rseq_off;
	struct tc_seqmap	tc_seqm;
	struct tc_seqmap	tc_rseqm;
	int			tc_sack_disable;
	int			tc_rto;
	void			*tc_timer;
	struct retransmit	*tc_retransmit;
	struct in_addr		tc_dst_ip;
	int			tc_dst_port;
	uint8_t			tc_nonce[MAX_NONCE];
	int			tc_nonce_len;
	struct tc_cipher_spec	tc_pub_cipher_list[MAX_CIPHERS];
	int			tc_pub_cipher_list_len;
	struct tc_scipher	tc_sym_cipher_list[MAX_CIPHERS];
	int                     tc_sym_cipher_list_len;
	struct stuff		tc_ss;
	struct stuff		tc_sid;
	struct stuff		tc_mk;
	struct stuff		tc_nk;
	struct tc_keyset	tc_key_current;
	struct tc_keyset	tc_key_next;
	struct tc_keyset	*tc_key_active;
	int			tc_role;
	int			tc_sym_ivlen;
	int			tc_sym_ivmode;
	int			tc_dir;
	int			tc_nocache;
	int			tc_dir_packet;
	int			tc_mac_opt_cache[DIR_OUT + 1];
	int			tc_csum;
	int			tc_verdict;
	void			*tc_last_ack_timer;
	unsigned int		tc_sent_bytes;
	unsigned char		tc_keygen;
	unsigned char		tc_keygentx;
	unsigned char		tc_keygenrx;
	unsigned int		tc_rekey_seq;
	unsigned char		tc_opt[40];
	int			tc_optlen;
	struct conn		*tc_conn;
	int			tc_app_support;
	uint64_t		tc_isn;
	uint64_t		tc_isn_peer;
	unsigned char		tc_init1[1500];
	int			tc_init1_len;
	unsigned char		tc_init2[1500];
	int			tc_init2_len;
	unsigned char		tc_pms[128];
	int			tc_pms_len;
	unsigned char		tc_eno[1500];
	int			tc_eno_len;
	int			tc_rdr_state;
	int			tc_rdr_connected;
	struct fd		*tc_rdr_fd;
	unsigned char		tc_rdr_buf[4096];
	int			tc_rdr_len;
	struct tc		*tc_rdr_peer;
	struct sockaddr_in	tc_rdr_addr;
	uint64_t		tc_rdr_tx;
	uint64_t		tc_rdr_rx;
	int			tc_rdr_inbound;
	int			tc_rdr_drop_sa;
};

enum {  
        TCOP_NONE               = 0x00,
        TCOP_HELLO		= 0x01,
	TCOP_HELLO_SUPPORT	= 0x02,
	TCOP_NEXTK2		= 0x05,
	TCOP_NEXTK2_SUPPORT	= 0x06,
	TCOP_INIT1		= 0x07,
	TCOP_INIT2		= 0x08,	
        TCOP_PKCONF             = 0x41,
        TCOP_PKCONF_SUPPORT	= 0x42,
	TCOP_REKEY		= 0x83,
        TCOP_NEXTK1		= 0x84,
        TCOP_NEXTK1_SUPPORT,
};

struct tc_subopt {
};

struct tco_rekeystream {
	uint8_t  tr_op;
	uint8_t  tr_key;
	uint32_t tr_seq;
} __attribute__ ((__packed__));

#define TCPOPT_SKEETER	16
#define TCPOPT_BUBBA	17
#define TCPOPT_MD5	19
#define TCPOPT_CRYPT	69
#define TCPOPT_MAC	70
#define TCPOPT_ENO	71

struct tcpopt_eno {
	uint8_t		 toe_kind;
	uint8_t		 toe_len;
	uint8_t		 toe_opts[0];
};

struct tcpopt_mac {
	uint8_t		tom_kind;
	uint8_t		tom_len;
	uint8_t		tom_data[0];
};

#define MACM_MAGIC 0x8000

struct mac_m {
        uint16_t        mm_magic;
        uint16_t        mm_len;
        uint8_t         mm_off;
        uint8_t         mm_flags;
        uint16_t        mm_urg;
        uint32_t        mm_seqhi;
        uint32_t        mm_seq;
};

struct mac_a {
        uint32_t        ma_ackhi;
        uint32_t        ma_ack;
};

enum {
	TC_INIT1 = 0x15101a0e,
	TC_INIT2 = 0x097105e0,
};

struct tc_init1 {
	uint32_t		i1_magic;
	uint32_t		i1_len;
	uint8_t			i1_nciphers;
	uint8_t			i1_data[0];
} __attribute__ ((__packed__));

struct tc_init2 {
	uint32_t		i2_magic;
	uint32_t		i2_len;
	uint8_t			i2_cipher;
	uint8_t			i2_data[0];
} __attribute__ ((__packed__));

struct tc_record {
	uint8_t	 tr_control;
	uint16_t tr_len;
	uint8_t  tr_data[0];
} __attribute__ ((__packed__));

struct cipher_list;

extern int  tcpcrypt_packet(void *packet, int len, int flags);
extern int  tcpcryptd_setsockopt(struct tcpcrypt_ctl *s, int opt, void *val,
			        unsigned int len);
extern int  tcpcryptd_getsockopt(struct tcpcrypt_ctl *s, int opt, void *val,
			        unsigned int *len);
extern void tcpcrypt_register_cipher(struct cipher_list *c);
extern void tcpcrypt_init(void);

extern struct tcphdr *get_tcp(struct ip *ip);

#endif /* __SRC_TCPCRYPT_H__ */
