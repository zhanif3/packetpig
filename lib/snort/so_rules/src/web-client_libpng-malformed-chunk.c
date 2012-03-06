/* 
Patrick Mullen <pmullen@sourcefire.com>

!!! XXX Not only does this not use the built-in detection function, I'm abusing the generator to create the structures for the chunk types that we'll be searching for manually later. XXX !!!

alert tcp $EXTERNAL_NET $HTTP_PORTS -> $HOME_NET any (msg:"WEB-CLIENT libpng malformed chunk denial of service attempt"; flow:to_client,established; content:"|89|PNG"; metadata: service http, policy security-ips drop, policy balanced-ips drop; reference:cve,2007-5269; classtype:attempted-dos; sid:14772; rev:1;)
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

//#define DEBUG
#ifdef DEBUG
#define DEBUG_WRAP(code) code
#else
#define DEBUG_WRAP(code)
#endif


/* declare detection functions */
int rule14772eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_client; */
static FlowFlags rule14772flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule14772option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule14772flow0
    }
};
// content:"|89|PNG"; 
static ContentInfo rule14772content1 = 
{
    (u_int8_t *) "|89|PNG", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule14772option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule14772content1
    }
};

/* references for sid 14772 */
/* reference: cve "2007-5269"; */
static RuleReference rule14772ref1 = 
{
    "cve", /* type */
    "2007-5269" /* value */
};

static RuleReference *rule14772refs[] =
{
    &rule14772ref1,
    NULL
};
/* metadata for sid 14772 */
/* metadata:service http, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule14772service1 = 
{
    "service http"
};


static RuleMetaData rule14772policy1 = 
{
    "policy balanced-ips drop"
};

static RuleMetaData rule14772policy2 = 
{
    "policy security-ips drop"
};


static RuleMetaData *rule14772metadata[] =
{
    &rule14772service1,
    &rule14772policy1,
    &rule14772policy2,
    NULL
};
RuleOption *rule14772options[] =
{
    &rule14772option0,
    &rule14772option1,
    NULL
};

Rule rule14772 = {
   
   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "$HTTP_PORTS", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       14772, /* sigid */
       3, /* revision */
   
       "attempted-dos", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "WEB-CLIENT libpng malformed chunk denial of service attempt",     /* message */
       rule14772refs /* ptr to references */
       ,rule14772metadata
   },
   rule14772options, /* ptr to rule options */
   &rule14772eval, /* DOES NOT use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule14772eval(void *p) {
   const u_int8_t *cursor_normal = 0, *beg_of_payload, *end_of_payload;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   const u_int8_t *end_of_chunk;
   u_int32_t chunk_size, chunk_name;

   DEBUG_WRAP(printf("rule14772eval libpng invalid chunks enter\n"));

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
    
   // flow:established, to_client;
   if(checkFlow(p, rule14772options[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   // content:"|89|PNG";
   if(contentMatch(p, rule14772options[1]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   /* Now we nominally know we're in a PNG file.  Jump over chunks until we find
      ones that we care about and check for terminators.
   */

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   cursor_normal += 4; // Skip to end of PNG signature

   while(cursor_normal + 8 < end_of_payload) { // Need room for size and signature
      chunk_size =  *cursor_normal++ << 24;
      chunk_size |= *cursor_normal++ << 16;
      chunk_size |= *cursor_normal++ << 8;
      chunk_size |= *cursor_normal++;

      //DEBUG_WRAP(printf("Chunk size 0x%04x\n", chunk_size));

      end_of_chunk = cursor_normal + chunk_size + 8; // +4 for chunk type, +4 for CRC

      // verify there is enough room, verify that the ptr did not wrap
      if (end_of_chunk >= end_of_payload || end_of_chunk <= cursor_normal)
         return RULE_NOMATCH;

      // Looking for tEXt, zTXt, iTXt, or sCAL.  First, we'll eliminate most
      // chunks by checking two different bytes.  If that passes, we're going
      // to speed things up by extracting the DWORD and then using a switch
      if(*(cursor_normal + 2) == 'X' || *(cursor_normal + 3) == 'L') {
         chunk_name =  *cursor_normal++ << 24;
         chunk_name |= *cursor_normal++ << 16;
         chunk_name |= *cursor_normal++ << 8;
         chunk_name |= *cursor_normal++;

         // At this point, the cursor is pointing to the start of chunk data
         switch(chunk_name) {
            case 0x74455874: // tEXt
               DEBUG_WRAP(printf("* Found tEXt chunk\n"));
               // Vuln is not seeing a NULL before the end of the chunk
               while(cursor_normal < end_of_chunk && *cursor_normal != 0x00) {
                  cursor_normal++;
               }

               // if no NULL was found, RULE_MATCH
               if(cursor_normal == end_of_chunk)
                  return RULE_MATCH;

               break;

            case 0x7a545874: // zTXt
               DEBUG_WRAP(printf("* Found zTXt chunk\n"));
               // Vuln is not seeing a NULL before the last byte of the chunk
               while((cursor_normal < (end_of_chunk - 1)) && *cursor_normal != 0x00) {
                  cursor_normal++;
               }

               // if no NULL was found, RULE_MATCH
               if(cursor_normal == (end_of_chunk - 1))
                  return RULE_MATCH;

               break;

            case 0x69545874: // iTXt
               DEBUG_WRAP(printf("* Found iTXt chunk\n"));
               // There need to be two NULLs for this one, unlike the others
               // Keyword
               while((cursor_normal < (end_of_chunk - 4)) && *cursor_normal != 0x00) {
                  cursor_normal++;
               }

               // if no NULL was found, RULE_MATCH
               if(cursor_normal == (end_of_chunk - 4))
                  return RULE_MATCH;

               cursor_normal += 2; // +1 comp flag +1 comp method

               // Language Tag
               while((cursor_normal < (end_of_chunk - 1)) && *cursor_normal != 0x00) {
                  cursor_normal++;
               }

               // if no NULL was found, RULE_MATCH
               if(cursor_normal == (end_of_chunk - 1))
                  return RULE_MATCH;
 
               // Text
               while((cursor_normal < end_of_chunk) && *cursor_normal != 0x00) {
                  cursor_normal++;
               }

               // if no NULL was found, RULE_MATCH
               if(cursor_normal == end_of_chunk)
                  return RULE_MATCH;

               break;

            case 0x7343414c: // sCAL
               DEBUG_WRAP(printf("* Found sCAL chunk\n"));
               // Vuln is not seeing a NULL before the end of the chunk,
               // after skipping the first chunk data byte
               cursor_normal++;

               while(cursor_normal < end_of_chunk && *cursor_normal != 0x00) {
                  cursor_normal++;
               }

               // if no NULL was found, RULE_MATCH
               if(cursor_normal == end_of_chunk)
                  return RULE_MATCH;

               break;

            default: // Ended up uninteresting
               DEBUG_WRAP(printf("Skipping %c%c%c%c chunk\n", *(cursor_normal), *(cursor_normal+1), *(cursor_normal+2), *(cursor_normal+3)));

         }      

      } else {
         DEBUG_WRAP(printf("Skipping %c%c%c%c chunk\n", *(cursor_normal), *(cursor_normal+1), *(cursor_normal+2), *(cursor_normal+3)));
      }

      cursor_normal = end_of_chunk;
   }        

   return RULE_NOMATCH;
}          

/*
Rule *rules[] = {
    &rule14772,
    NULL
};
*/

