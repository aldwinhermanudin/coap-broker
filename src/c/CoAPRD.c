#include "CoAPRD.h"

rd_t *resources = NULL;

rd_t *rd_new(void) {
  rd_t *rd;
  rd = (rd_t *)coap_malloc(sizeof(rd_t));
  if (rd)
    memset(rd, 0, sizeof(rd_t));

  return rd;
}

void rd_delete(rd_t *rd) {
  if (rd) {
    coap_free(rd->data.s);
    coap_free(rd);
  }
}

void hnd_get_resource(coap_context_t  *ctx ,
                 struct coap_resource_t *resource,
                 const coap_endpoint_t *local_interface ,
                 coap_address_t *peer ,
                 coap_pdu_t *request ,
                 str *token ,
                 coap_pdu_t *response) {
  rd_t *rd = NULL;
  unsigned char buf[3];

  HASH_FIND(hh, resources, resource->key, sizeof(coap_key_t), rd);

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf,
                                        COAP_MEDIATYPE_APPLICATION_LINK_FORMAT),
                                        buf);

  if (rd && rd->etag_len)
    coap_add_option(response, COAP_OPTION_ETAG, rd->etag_len, rd->etag);

  if (rd && rd->data.s)
    coap_add_data(response, rd->data.length, rd->data.s);
}

void hnd_put_resource(coap_context_t  *ctx ,
                 struct coap_resource_t *resource ,
                 const coap_endpoint_t *local_interface ,
                 coap_address_t *peer ,
                 coap_pdu_t *request ,
                 str *token ,
                 coap_pdu_t *response) {
#if 1
  response->hdr->code = COAP_RESPONSE_CODE(501);
#else /* FIXME */
  coap_opt_iterator_t opt_iter;
  coap_opt_t *token, *etag;
  coap_pdu_t *response;
  size_t size = sizeof(coap_hdr_t);
  int type = (request->hdr->type == COAP_MESSAGE_CON)
    ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON;
  rd_t *rd = NULL;
  unsigned char code;     /* result code */
  unsigned char *data;
  str tmp;

  HASH_FIND(hh, resources, resource->key, sizeof(coap_key_t), rd);
  if (rd) {
    /* found resource object, now check Etag */
    etag = coap_check_option(request, COAP_OPTION_ETAG, &opt_iter);
    if (!etag || (COAP_OPT_LENGTH(etag) != rd->etag_len)
        || memcmp(COAP_OPT_VALUE(etag), rd->etag, rd->etag_len) != 0) {

      if (coap_get_data(request, &tmp.length, &data)) {

        tmp.s = (unsigned char *)coap_malloc(tmp.length);
        if (!tmp.s) {
          debug("hnd_put_rd: cannot allocate storage for new rd\n");
          code = COAP_RESPONSE_CODE(503);
          goto finish;
        }

        coap_free(rd->data.s);
        rd->data.s = tmp.s;
        rd->data.length = tmp.length;
        memcpy(rd->data.s, data, rd->data.length);
      }
    }

    if (etag) {
      rd->etag_len = min(COAP_OPT_LENGTH(etag), sizeof(rd->etag));
      memcpy(rd->etag, COAP_OPT_VALUE(etag), rd->etag_len);
    }

    code = COAP_RESPONSE_CODE(204);
    /* FIXME: update lifetime */

    } else {

    code = COAP_RESPONSE_CODE(503);
  }

  finish:
  /* FIXME: do not create a new response but use the old one instead */
  response = coap_pdu_init(type, code, request->hdr->id, size);

  if (!response) {
    debug("cannot create response for message %d\n", request->hdr->id);
    return;
  }

  if (request->hdr->token_length)
    coap_add_token(response, request->hdr->token_length, request->hdr->token);

  if (coap_send(ctx, peer, response) == COAP_INVALID_TID) {
    debug("hnd_get_rd: cannot send response for message %d\n",
    request->hdr->id);
  }
  coap_delete_pdu(response);
#endif
}

void hnd_delete_resource(coap_context_t  *ctx,
                    struct coap_resource_t *resource,
                    const coap_endpoint_t *local_interface ,
                    coap_address_t *peer ,
                    coap_pdu_t *request ,
                    str *token ,
                    coap_pdu_t *response) {
  rd_t *rd = NULL;

  HASH_FIND(hh, resources, resource->key, sizeof(coap_key_t), rd);
  if (rd) {
    HASH_DELETE(hh, resources, rd);
    rd_delete(rd);
  }
  /* FIXME: link attributes for resource have been created dynamically
   * using coap_malloc() and must be released. */
  coap_delete_resource(ctx, resource->key);

  response->hdr->code = COAP_RESPONSE_CODE(202);
}

void hnd_get_rd(coap_context_t  *ctx ,
           struct coap_resource_t *resource ,
           const coap_endpoint_t *local_interface ,
           coap_address_t *peer ,
           coap_pdu_t *request ,
           str *token ,
           coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf,
                                        COAP_MEDIATYPE_APPLICATION_LINK_FORMAT),
                                        buf);

  coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);
}

int parse_param(unsigned char *search,
            size_t search_len,
            unsigned char *data,
            size_t data_len,
            str *result) {

  if (result)
    memset(result, 0, sizeof(str));

  if (!search_len)
    return 0;

  while (search_len <= data_len) {

    /* handle parameter if found */
    if (memcmp(search, data, search_len) == 0) {
      data += search_len;
      data_len -= search_len;

      /* key is only valid if we are at end of string or delimiter follows */
      if (!data_len || *data == '=' || *data == '&') {
        while (data_len && *data != '=') {
          ++data; --data_len;
        }

        if (data_len > 1 && result) {
          /* value begins after '=' */

          result->s = ++data;
          while (--data_len && *data != '&') {
            ++data; result->length++;
          }
        }

        return 1;
      }
    }

    /* otherwise proceed to next */
    while (--data_len && *data++ != '&')
      ;
  }

  return 0;
}

void add_source_address(struct coap_resource_t *resource,
                   coap_address_t *peer) {
#define BUFSIZE 64
  char *buf;
  size_t n = 1;

  buf = (char *)coap_malloc(BUFSIZE);
  if (!buf)
    return;

  buf[0] = '"';

  switch(peer->addr.sa.sa_family) {

  case AF_INET:
    /* FIXME */
    break;

  case AF_INET6:
    n += snprintf(buf + n, BUFSIZE - n,
      "[%02x%02x:%02x%02x:%02x%02x:%02x%02x" \
      ":%02x%02x:%02x%02x:%02x%02x:%02x%02x]",
      peer->addr.sin6.sin6_addr.s6_addr[0],
      peer->addr.sin6.sin6_addr.s6_addr[1],
      peer->addr.sin6.sin6_addr.s6_addr[2],
      peer->addr.sin6.sin6_addr.s6_addr[3],
      peer->addr.sin6.sin6_addr.s6_addr[4],
      peer->addr.sin6.sin6_addr.s6_addr[5],
      peer->addr.sin6.sin6_addr.s6_addr[6],
      peer->addr.sin6.sin6_addr.s6_addr[7],
      peer->addr.sin6.sin6_addr.s6_addr[8],
      peer->addr.sin6.sin6_addr.s6_addr[9],
      peer->addr.sin6.sin6_addr.s6_addr[10],
      peer->addr.sin6.sin6_addr.s6_addr[11],
      peer->addr.sin6.sin6_addr.s6_addr[12],
      peer->addr.sin6.sin6_addr.s6_addr[13],
      peer->addr.sin6.sin6_addr.s6_addr[14],
      peer->addr.sin6.sin6_addr.s6_addr[15]);

    if (peer->addr.sin6.sin6_port != htons(COAP_DEFAULT_PORT)) {
      n +=
      snprintf(buf + n, BUFSIZE - n, ":%d", peer->addr.sin6.sin6_port);
    }
    break;
    default:
    ;
  }

  if (n < BUFSIZE)
    buf[n++] = '"';

  coap_add_attr(resource,
                (unsigned char *)"A",
                1,
                (unsigned char *)buf,
                n,
                COAP_ATTR_FLAGS_RELEASE_VALUE);
#undef BUFSIZE
}

rd_t *make_rd(coap_address_t *peer , coap_pdu_t *pdu) {
  rd_t *rd;
  unsigned char *data;
  coap_opt_iterator_t opt_iter;
  coap_opt_t *etag;

  rd = rd_new();

  if (!rd) {
    debug("hnd_get_rd: cannot allocate storage for rd\n");
    return NULL;
  }

  if (coap_get_data(pdu, &rd->data.length, &data)) {
    rd->data.s = (unsigned char *)coap_malloc(rd->data.length);
    if (!rd->data.s) {
      debug("hnd_get_rd: cannot allocate storage for rd->data\n");
      rd_delete(rd);
      return NULL;
    }
    memcpy(rd->data.s, data, rd->data.length);
  }

  etag = coap_check_option(pdu, COAP_OPTION_ETAG, &opt_iter);
  if (etag) {
    rd->etag_len = min(COAP_OPT_LENGTH(etag), sizeof(rd->etag));
    memcpy(rd->etag, COAP_OPT_VALUE(etag), rd->etag_len);
  }

  return rd;
}

void hnd_post_rd(coap_context_t  *ctx,
            struct coap_resource_t *resource ,
            const coap_endpoint_t *local_interface ,
            coap_address_t *peer,
            coap_pdu_t *request,
            str *token ,
            coap_pdu_t *response) {
  coap_resource_t *r;
  coap_opt_iterator_t opt_iter;
  coap_opt_t *query;
#define LOCSIZE 68
  unsigned char *loc;
  size_t loc_size;
  str h = {0, NULL}, ins = {0, NULL}, rt = {0, NULL}, lt = {0, NULL}; /* store query parameters */
  unsigned char *buf;

  loc = (unsigned char *)coap_malloc(LOCSIZE);
  if (!loc) {
    response->hdr->code = COAP_RESPONSE_CODE(500);
    return;
  }
  memcpy(loc, RD_ROOT_STR, RD_ROOT_SIZE);

  loc_size = RD_ROOT_SIZE;
  loc[loc_size++] = '/';

  /* store query parameters for later use */
  query = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
  if (query) {
    parse_param((unsigned char *)"h", 1,
    COAP_OPT_VALUE(query), COAP_OPT_LENGTH(query), &h);
    parse_param((unsigned char *)"ins", 3,
    COAP_OPT_VALUE(query), COAP_OPT_LENGTH(query), &ins);
    parse_param((unsigned char *)"lt", 2,
    COAP_OPT_VALUE(query), COAP_OPT_LENGTH(query), &lt);
    parse_param((unsigned char *)"rt", 2,
    COAP_OPT_VALUE(query), COAP_OPT_LENGTH(query), &rt);
  }

  if (h.length) {   /* client has specified a node name */
    memcpy(loc + loc_size, h.s, min(h.length, LOCSIZE - loc_size - 1));
    loc_size += min(h.length, LOCSIZE - loc_size - 1);

    if (ins.length && loc_size > 1) {
      loc[loc_size++] = '-';
      memcpy((char *)(loc + loc_size),
      ins.s, min(ins.length, LOCSIZE - loc_size - 1));
      loc_size += min(ins.length, LOCSIZE - loc_size - 1);
    }

  } else {      /* generate node identifier */
    loc_size +=
      snprintf((char *)(loc + loc_size), LOCSIZE - loc_size - 1,
               "%x", request->hdr->id);

    if (loc_size > 1) {
      if (ins.length) {
        loc[loc_size++] = '-';
        memcpy((char *)(loc + loc_size),
                ins.s,
                min(ins.length, LOCSIZE - loc_size - 1));
        loc_size += min(ins.length, LOCSIZE - loc_size - 1);
      } else {
        coap_tick_t now;
        coap_ticks(&now);

        loc_size += snprintf((char *)(loc + loc_size),
                             LOCSIZE - loc_size - 1,
                             "-%x",
                             (unsigned int)(now & (unsigned int)-1));
      }
    }
  }

  /* TODO:
   *   - use lt to check expiration
   */

  r = coap_resource_init(loc, loc_size, COAP_RESOURCE_FLAGS_RELEASE_URI);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_resource);
  coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_resource);
  coap_register_handler(r, COAP_REQUEST_DELETE, hnd_delete_resource);

  if (ins.s) {
    buf = (unsigned char *)coap_malloc(ins.length + 2);
    if (buf) {
      /* add missing quotes */
      buf[0] = '"';
      memcpy(buf + 1, ins.s, ins.length);
      buf[ins.length + 1] = '"';
      coap_add_attr(r,
                    (unsigned char *)"ins",
                    3,
                    buf,
                    ins.length + 2,
                    COAP_ATTR_FLAGS_RELEASE_VALUE);
    }
  }

  if (rt.s) {
    buf = (unsigned char *)coap_malloc(rt.length + 2);
    if (buf) {
      /* add missing quotes */
      buf[0] = '"';
      memcpy(buf + 1, rt.s, rt.length);
      buf[rt.length + 1] = '"';
      coap_add_attr(r,
                    (unsigned char *)"rt",
                    2,
                    buf,
                    rt.length + 2,COAP_ATTR_FLAGS_RELEASE_VALUE);
    }
  }

  add_source_address(r, peer);

  {
    rd_t *rd;
    rd = make_rd(peer, request);
    if (rd) {
      coap_hash_path(loc, loc_size, rd->key);
      HASH_ADD(hh, resources, key, sizeof(coap_key_t), rd);
    } else {
      /* FIXME: send error response and delete r */
    }
  }

  coap_add_resource(ctx, r);


  /* create response */

  response->hdr->code = COAP_RESPONSE_CODE(201);

  { /* split path into segments and add Location-Path options */
    unsigned char _b[LOCSIZE];
    unsigned char *b = _b;
    size_t buflen = sizeof(_b);
    int nseg;

    nseg = coap_split_path(loc, loc_size, b, &buflen);
    while (nseg--) {
      coap_add_option(response,
                      COAP_OPTION_LOCATION_PATH,
                      COAP_OPT_LENGTH(b),
                      COAP_OPT_VALUE(b));
      b += COAP_OPT_SIZE(b);
    }
  }
}

void init_rd_resources(coap_context_t *ctx) {
  coap_resource_t *r;

  r = coap_resource_init(RD_ROOT_STR, RD_ROOT_SIZE, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_rd);
  coap_register_handler(r, COAP_REQUEST_POST, hnd_post_rd);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"40", 2, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"core.rd\"", 9, 0);
  coap_add_attr(r, (unsigned char *)"ins", 2, (unsigned char *)"\"default\"", 9, 0);

  coap_add_resource(ctx, r);

}
