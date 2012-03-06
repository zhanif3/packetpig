/* DOES NOT USE BUILT-IN DETECTION FUNCTIONALITY!
alert tcp $EXTERNAL_NET any -> $HOME_NET 389 (msg:"EXPLOIT IBM Tivoli directory server LDAP identifier buffer overflow attempt"; flow:to_server,established; content:"|30|"; depth:1; reference:cve,2006-0717; reference:bugtraq,16593; reference:url,www-1.ibm.com/support/docview.wss?uid=swq21230820; classtype:attempted-admin; sid:13418; rev:1;)

Upon further review this exploit can actually result in code execution, but it was previously put in as a dos, so there it stays.

*/
/*
 * Use at your own risk.
 *
 * Copyright (C) 2005-2008 Sourcefire, Inc.
 * 
 * This file is autogenerated via rules2c, by Brian Caswell <bmc@sourcefire.com>
 */


#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "dos_ber.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_SO(code) code
#else
#define DEBUG_SO(code)
#endif

/* declare detection functions */
int rule13418eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_server; */
static FlowFlags rule13418flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule13418option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule13418flow0
    }
};
// content:"0", depth 1; 
static ContentInfo rule13418content1 = 
{
    (u_int8_t *) "0", /* pattern (now in snort content format) */
    1, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule13418option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule13418content1
    }
};

/* references for sid 13418 */
/* reference: bugtraq "16593"; */
static RuleReference rule13418ref1 = 
{
    "bugtraq", /* type */
    "16593" /* value */
};

/* reference: cve "2006-0717"; */
static RuleReference rule13418ref2 = 
{
    "cve", /* type */
    "2006-0717" /* value */
};

/* reference: url "www-1.ibm.com/support/docview.wss?uid=swq21230820"; */
static RuleReference rule13418ref3 = 
{
    "url", /* type */
    "www-1.ibm.com/support/docview.wss?uid=swq21230820" /* value */
};

/* reference:cve,2011-0917; */
static RuleReference rule13418ref4 =
{
    "cve", /* type */
    "2011-0917" /* value */
};

static RuleReference *rule13418refs[] =
{
    &rule13418ref1,
    &rule13418ref2,
    &rule13418ref3,
    &rule13418ref4,
    NULL
};

/* metadata for sid 13418 */
/* metadata:; */


static RuleMetaData *rule13418metadata[] =
{
    NULL
};

RuleOption *rule13418options[] =
{
    &rule13418option0,
    &rule13418option1,
    NULL
};

Rule rule13418 = {
   
   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "389", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       13418, /* sigid */
       5, /* revision */
   
       "attempted-dos", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "DOS IBM Tivoli Director LDAP server invalid DN message buffer overflow attempt",     /* message */
       rule13418refs /* ptr to references */
       ,rule13418metadata
   },
   rule13418options, /* ptr to rule options */
   &rule13418eval, /* use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule13418eval(void *p) {
   const u_int8_t *cursor_normal = 0, *end_of_payload;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   int retval;
   BER_ELEMENT element;

   DEBUG_SO(printf("rule13418eval (tivoli identifier overflow) enter\n"));

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
    
   // flow:established, to_server;
   if(checkFlow(p, rule13418options[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   // Universal Sequence
   if(ber_point_to_data(sp, &cursor_normal, 0x30) < 0)
      return RULE_NOMATCH;
 
   // message ID
   if(ber_skip_element(sp, &cursor_normal, 0x02) < 0) 
      return RULE_NOMATCH;

   // bind request
   if(ber_point_to_data(sp, &cursor_normal, 0x60) < 0)
      return RULE_NOMATCH;

   // ldap version
   if(ber_skip_element(sp, &cursor_normal, 0x02) < 0)
      return RULE_NOMATCH;

   // Here's the meat
   retval = ber_get_element(sp, cursor_normal, &element);

   DEBUG_SO(printf("found identifiers string.  checking values\n"));

   /* Here is the actual exploit detection.   Technically, the exploit is
      for when the string length is exactly 0xFFFFFFFF bytes long, which
      results in an integer overflow.  However, we're going to be more
      stringent and flag if the string is longer than 0xFFFF bytes long.
      Why would a DN be more than 65k bytes?
   */

   /* retval == -2 means element data is > 0xFFFFFFFF bytes long */
   if(element.type == 0x04 && ((element.data_len > 0xFFFF) || (retval == -2)))
      return RULE_MATCH;

   // We have a sample that has an odd structure -- 0-length string (type 0x04) 
   // followed by 0x80 and size with the string there.  Is this valid??  
   if(ber_skip_element(sp, &cursor_normal, 0x04) < 0)
      return RULE_NOMATCH;

   retval = ber_get_element(sp, cursor_normal, &element);
   DEBUG_SO(printf("Funky format, but found identifiers string.  checking values\n"));

   if(element.type == 0x80 && ((element.data_len > 0xFFFF) || (retval == -2)))
      return RULE_MATCH;

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule13418,
    NULL
};
*/

