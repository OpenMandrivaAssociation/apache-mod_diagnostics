/*
  mod_diagnostics

  Copyright (C) 2003, Nick Kew <nick@webthing.com>

  This is free software.  You may use and redistribute it under
  the terms of the Apache License at
  http://www.apache.org/LICENSE.txt
*/

/*
  mod_diagnostics: print diagnostic and debug information on data
  (and metadata) passing through an Apache Filter chain.
 
  Insert a mod_diagnostics filter anywhere you want to watch traffic.
  See below for registered input and output filter names.

  Two filters are defined for each level, so that you can insert
  mod_diagnostics before and after any module you are investigating
  or debugging.
*/

#include <httpd.h>
#include <http_config.h>
#include <http_core.h>
#include <http_log.h>

module AP_MODULE_DECLARE_DATA diagnostic_filter_module ;

static void diagnostic_log(ap_filter_t* f, apr_bucket* b) {

/* use the connection pool, so it works with all filter types
   (Request may not be valid in a connection or network filter)

   This doesn't work with APLOG_DEBUG (looks like a bug in log.c
   around line 409 in 2.0.44), so we use APLOG_NOTICE.  This is
   worth updating if httpd gets fixed.
*/
  ap_log_perror(APLOG_MARK, APLOG_NOTICE, 0, f->c->pool,
	"  %s %s: %d bytes", f->frec->name, b->type->name, b->length) ;
}
static int diagnostic_ofilter (ap_filter_t* f, apr_bucket_brigade* bb) {
  apr_bucket* b ;
  ap_log_perror(APLOG_MARK, APLOG_NOTICE, 0, f->c->pool, f->frec->name) ;

  for (	b = APR_BRIGADE_FIRST(bb) ;
	b != APR_BRIGADE_SENTINEL(bb) ;
	b = APR_BUCKET_NEXT(b) )
    diagnostic_log(f, b) ;

  return ap_pass_brigade(f->next, bb) ;
}
static const char* getmode(ap_input_mode_t mode) {
  switch ( mode ) {
	case AP_MODE_READBYTES: return "READBYTES" ;
	case AP_MODE_GETLINE: return "GETLINE" ;
	case AP_MODE_EATCRLF: return "EATCRLF" ;
	case AP_MODE_SPECULATIVE: return "SPECULATIVE" ;
	case AP_MODE_EXHAUSTIVE: return "EXHAUSTIVE" ;
	case AP_MODE_INIT: return "INIT" ;
  }
  return "(unknown)" ;
}
#define gettype(block) ((block) == APR_BLOCK_READ) ? "blocking" : "non-blocking"
static int diagnostic_ifilter (ap_filter_t* f, apr_bucket_brigade* bb,
	ap_input_mode_t mode, apr_read_type_e block, apr_off_t readbytes) {

  apr_bucket* b ;
  apr_status_t ret ;

  ap_log_perror(APLOG_MARK, APLOG_NOTICE, 0, f->c->pool, 
	"%s: mode %s; %s; %ld bytes", f->frec->name,
	getmode(mode), gettype(block), readbytes) ;

  if ( ret = ap_get_brigade(f->next, bb, mode, block, readbytes) ,
	ret == APR_SUCCESS )
    for ( b = APR_BRIGADE_FIRST(bb) ;
	b != APR_BRIGADE_SENTINEL(bb) ;
	b = APR_BUCKET_NEXT(b) )
      diagnostic_log(f, b) ;
  else
    ap_log_perror(APLOG_MARK, APLOG_NOTICE, 0, f->c->pool,
	"%s: ap_get_brigade returned %d", f->frec->name, ret) ;

  return ret ;
}

typedef struct {
  int bytes ;
} split_cfg ;
static apr_status_t split_filter(ap_filter_t* f, apr_bucket_brigade* bb) {
  apr_bucket* b ;
  const char* buf ;
  apr_size_t bytes ;
  split_cfg* cfg = ap_get_module_config(f->r->per_dir_config,
	&diagnostic_filter_module) ;
  for (b = APR_BRIGADE_FIRST(bb) ;
	b != APR_BRIGADE_SENTINEL(bb) ;
	b = APR_BUCKET_NEXT(b) ) {
    if (apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ) == APR_SUCCESS) {
      while (bytes > cfg->bytes) {
	apr_bucket_split(b, cfg->bytes) ;
	b = APR_BUCKET_NEXT(b) ;
	bytes -= cfg->bytes ;
      }
    }
  }
  return ap_pass_brigade(f->next, bb) ;
}
static const command_rec split_cmds[] = {
  AP_INIT_TAKE1("SplitSize", ap_set_int_slot,
	(void*)APR_OFFSETOF(split_cfg, bytes), RSRC_CONF|ACCESS_CONF,
	"Size at which to split buckets") ,
  {NULL}
};
static void* split_cr_cfg(apr_pool_t* pool, char* x) {
  split_cfg* ret = apr_palloc(pool, sizeof(split_cfg)) ;
  ret->bytes = 8192;
  return ret ;
}


#define ofilter_init NULL
#define ifilter_init NULL

static void diagnostic_hooks(apr_pool_t* p) {
/* by registering twice under each phase, we can insert filters
   BEFORE and AFTER one we are debugging, and distinguish between them

   I don't think this makes much sense at the network level, but
   we'll do it anyway: nothing to lose!
*/
  ap_register_output_filter("o-resource-1", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_RESOURCE) ;
  ap_register_output_filter("o-resource-2", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_RESOURCE) ;
  ap_register_output_filter("o-content-1", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_CONTENT_SET) ;
  ap_register_output_filter("o-content-2", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_CONTENT_SET) ;
  ap_register_output_filter("o-protocol-1", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_PROTOCOL) ;
  ap_register_output_filter("o-protocol-2", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_PROTOCOL) ;
  ap_register_output_filter("o-transcode-1", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_TRANSCODE) ;
  ap_register_output_filter("o-transcode-2", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_TRANSCODE) ;
  ap_register_output_filter("o-connection-1", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_CONNECTION) ;
  ap_register_output_filter("o-connection-2", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_CONNECTION) ;
  ap_register_output_filter("o-network-1", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_NETWORK) ;
  ap_register_output_filter("o-network-2", diagnostic_ofilter,
	ofilter_init, AP_FTYPE_NETWORK) ;

  ap_register_input_filter("i-resource-1", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_RESOURCE-1) ;
  ap_register_input_filter("i-resource-2", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_RESOURCE+1) ;
  ap_register_input_filter("i-content-1", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_CONTENT_SET) ;
  ap_register_input_filter("i-content-2", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_CONTENT_SET) ;
  ap_register_input_filter("i-protocol-1", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_PROTOCOL) ;
  ap_register_input_filter("i-protocol-2", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_PROTOCOL) ;
  ap_register_input_filter("i-transcode-1", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_TRANSCODE) ;
  ap_register_input_filter("i-transcode-2", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_TRANSCODE) ;
  ap_register_input_filter("i-connection-1", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_CONNECTION) ;
  ap_register_input_filter("i-connection-2", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_CONNECTION) ;
  ap_register_input_filter("i-network-1", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_NETWORK) ;
  ap_register_input_filter("i-network-2", diagnostic_ifilter,
	ifilter_init, AP_FTYPE_NETWORK) ;

  ap_register_output_filter("split-filter", split_filter,
	NULL, AP_FTYPE_RESOURCE) ;
}
module AP_MODULE_DECLARE_DATA diagnostic_filter_module = {
	STANDARD20_MODULE_STUFF,
	split_cr_cfg,
	NULL,
	NULL,
	NULL,
	split_cmds,
	diagnostic_hooks
} ;
