/*
 * Copyright 2016 Hans Leidekker for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>
#include "windows.h"
#include "winsock2.h"
#include "webservices.h"
#include "wine/test.h"

static inline void set_field_desc( WS_FIELD_DESCRIPTION *desc, WS_FIELD_MAPPING mapping,
                                   WS_XML_STRING *localname, WS_XML_STRING *ns, WS_TYPE type,
                                   void *type_desc, ULONG offset, ULONG options, ULONG count_offset,
                                   WS_XML_STRING *item_localname, WS_XML_STRING *item_ns )
{
    memset( desc, 0, sizeof(*desc) );
    desc->mapping         = mapping;
    desc->localName       = localname;
    desc->ns              = ns;
    desc->type            = type;
    desc->typeDescription = type_desc;
    desc->offset          = offset;
    desc->options         = options;
    desc->countOffset     = count_offset;
    desc->itemLocalName   = item_localname;
    desc->itemNs          = item_ns;
}

static inline void set_struct_desc( WS_STRUCT_DESCRIPTION *desc, ULONG size, ULONG alignment,
                                    WS_FIELD_DESCRIPTION **fields, ULONG count, WS_XML_STRING *localname,
                                    WS_XML_STRING *ns, ULONG options )
{
    memset( desc, 0, sizeof(*desc) );
    desc->size          = size;
    desc->alignment     = alignment;
    desc->fields        = fields;
    desc->fieldCount    = count;
    desc->typeLocalName = localname;
    desc->typeNs        = ns;
    desc->structOptions = options;
}

static inline void set_elem_desc( WS_ELEMENT_DESCRIPTION *desc, WS_XML_STRING *localname, WS_XML_STRING *ns,
                                  WS_TYPE type, void *type_desc )
{
    desc->elementLocalName = localname;
    desc->elementNs        = ns;
    desc->type             = type;
    desc->typeDescription  = type_desc;
}

static inline void set_msg_desc( WS_MESSAGE_DESCRIPTION *desc, WS_XML_STRING *action,
                                 WS_ELEMENT_DESCRIPTION *elem_desc )
{
    desc->action                 = action;
    desc->bodyElementDescription = elem_desc;
}

static inline void set_param_desc( WS_PARAMETER_DESCRIPTION *desc, WS_PARAMETER_TYPE type,
                                   USHORT input_index, USHORT output_index )
{
    desc->parameterType      = type;
    desc->inputMessageIndex  = input_index;
    desc->outputMessageIndex = output_index;
}

static inline void set_op_desc( WS_OPERATION_DESCRIPTION *desc, WS_MESSAGE_DESCRIPTION *input_msg,
                                WS_MESSAGE_DESCRIPTION *output_msg, ULONG count,
                                WS_PARAMETER_DESCRIPTION *param_desc )
{
    memset( desc, 0, sizeof(*desc) );
    desc->versionInfo              = 1;
    desc->inputMessageDescription  = input_msg;
    desc->outputMessageDescription = output_msg;
    desc->parameterCount           = count;
    desc->parameterDescription     = param_desc;
}

static void test_WsCreateServiceProxy(void)
{
    HRESULT hr;
    WS_SERVICE_PROXY *proxy;
    WS_SERVICE_PROXY_STATE state;
    ULONG size, value;

    hr = WsCreateServiceProxy( WS_CHANNEL_TYPE_REQUEST, WS_HTTP_CHANNEL_BINDING, NULL, NULL,
                               0, NULL, 0, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    proxy = NULL;
    hr = WsCreateServiceProxy( WS_CHANNEL_TYPE_REQUEST, WS_HTTP_CHANNEL_BINDING, NULL, NULL,
                               0, NULL, 0, &proxy, NULL );
    ok( hr == S_OK, "got %08x\n", hr );
    ok( proxy != NULL, "proxy not set\n" );

    /* write-only property */
    value = 0xdeadbeef;
    size = sizeof(value);
    hr = WsGetServiceProxyProperty( proxy, WS_PROXY_PROPERTY_CALL_TIMEOUT, &value, size, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    state = 0xdeadbeef;
    size = sizeof(state);
    hr = WsGetServiceProxyProperty( proxy, WS_PROXY_PROPERTY_STATE, &state, size, NULL );
    ok( hr == S_OK, "got %08x\n", hr );
    ok( state == WS_SERVICE_PROXY_STATE_CREATED, "got %u\n", state );

    WsFreeServiceProxy( proxy );
}

static void test_WsCreateServiceProxyFromTemplate(void)
{
    HRESULT hr;
    WS_SERVICE_PROXY *proxy;
    WS_HTTP_POLICY_DESCRIPTION policy;

    hr = WsCreateServiceProxyFromTemplate( WS_CHANNEL_TYPE_REQUEST, NULL, 0, WS_HTTP_BINDING_TEMPLATE_TYPE,
                                           NULL, 0, NULL, 0, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    hr = WsCreateServiceProxyFromTemplate( WS_CHANNEL_TYPE_REQUEST, NULL, 0, WS_HTTP_BINDING_TEMPLATE_TYPE,
                                           NULL, 0, NULL, 0, &proxy, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    memset( &policy, 0, sizeof(policy) );
    proxy = NULL;
    hr = WsCreateServiceProxyFromTemplate( WS_CHANNEL_TYPE_REQUEST, NULL, 0, WS_HTTP_BINDING_TEMPLATE_TYPE,
                                           NULL, 0, &policy, sizeof(policy), &proxy, NULL );
    ok( hr == S_OK, "got %08x\n", hr );
    ok( proxy != NULL, "proxy not set\n" );

    WsFreeServiceProxy( proxy );
}

static void test_WsOpenServiceProxy(void)
{
    WCHAR url[] = {'h','t','t','p',':','/','/','l','o','c','a','l','h','o','s','t','/'};
    HRESULT hr;
    WS_SERVICE_PROXY *proxy;
    WS_HTTP_POLICY_DESCRIPTION policy;
    WS_ENDPOINT_ADDRESS addr;

    memset( &policy, 0, sizeof(policy) );
    hr = WsCreateServiceProxyFromTemplate( WS_CHANNEL_TYPE_REQUEST, NULL, 0, WS_HTTP_BINDING_TEMPLATE_TYPE,
                                           NULL, 0, &policy, sizeof(policy), &proxy, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    addr.url.length = sizeof(url)/sizeof(url[0]);
    addr.url.chars  = url;
    addr.headers    = NULL;
    addr.extensions = NULL;
    addr.identity   = NULL;
    hr = WsOpenServiceProxy( proxy, &addr, NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    hr = WsCloseServiceProxy( proxy , NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    WsFreeServiceProxy( proxy );
}

static HRESULT create_channel( int port, WS_CHANNEL **ret )
{
    static const WCHAR fmt[] =
        {'h','t','t','p',':','/','/','1','2','7','.','0','.','0','.','1',':','%','u',0};
    WS_CHANNEL_PROPERTY prop[2];
    WS_ENVELOPE_VERSION env_version = WS_ENVELOPE_VERSION_SOAP_1_1;
    WS_ADDRESSING_VERSION addr_version = WS_ADDRESSING_VERSION_TRANSPORT;
    WS_CHANNEL *channel;
    WS_ENDPOINT_ADDRESS addr;
    WCHAR buf[64];
    HRESULT hr;

    prop[0].id        = WS_CHANNEL_PROPERTY_ENVELOPE_VERSION;
    prop[0].value     = &env_version;
    prop[0].valueSize = sizeof(env_version);

    prop[1].id        = WS_CHANNEL_PROPERTY_ADDRESSING_VERSION;
    prop[1].value     = &addr_version;
    prop[1].valueSize = sizeof(addr_version);

    *ret = NULL;
    hr = WsCreateChannel( WS_CHANNEL_TYPE_REQUEST, WS_HTTP_CHANNEL_BINDING, prop, 2, NULL, &channel, NULL );
    if (hr != S_OK) return hr;

    addr.url.length = wsprintfW( buf, fmt, port );
    addr.url.chars  = buf;
    addr.headers    = NULL;
    addr.extensions = NULL;
    addr.identity   = NULL;
    hr = WsOpenChannel( channel, &addr, NULL, NULL );
    if (hr == S_OK) *ret = channel;
    else WsFreeChannel( channel );
    return hr;
}

static const char req_test1[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>"
    "<req_test1 xmlns=\"ns\">-1</req_test1>"
    "</s:Body></s:Envelope>";

static const char resp_test1[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>"
    "<resp_test1 xmlns=\"ns\">-2</resp_test1>"
    "</s:Body></s:Envelope>";

static void test_WsSendMessage( int port, WS_XML_STRING *action )
{
    WS_XML_STRING req = {9, (BYTE *)"req_test1"}, ns = {2, (BYTE *)"ns"};
    WS_CHANNEL *channel;
    WS_MESSAGE *msg;
    WS_ELEMENT_DESCRIPTION body;
    WS_MESSAGE_DESCRIPTION desc;
    INT32 val = -1;
    HRESULT hr;

    hr = create_channel( port, &channel );
    ok( hr == S_OK, "got %08x\n", hr );

    hr = WsCreateMessageForChannel( channel, NULL, 0, &msg, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    set_elem_desc( &body, &req, &ns, WS_INT32_TYPE, NULL );
    set_msg_desc( &desc, action, &body );
    hr = WsSendMessage( NULL, msg, &desc, WS_WRITE_REQUIRED_VALUE, &val, sizeof(val), NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    hr = WsSendMessage( channel, NULL, &desc, WS_WRITE_REQUIRED_VALUE, &val, sizeof(val), NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    hr = WsSendMessage( channel, msg, NULL, WS_WRITE_REQUIRED_VALUE, &val, sizeof(val), NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    hr = WsSendMessage( channel, msg, &desc, WS_WRITE_REQUIRED_VALUE, &val, sizeof(val), NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    hr = WsCloseChannel( channel, NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    WsFreeChannel( channel );
    WsFreeMessage( msg );
}

static void test_WsReceiveMessage( int port )
{
    WS_XML_STRING req = {9, (BYTE *)"req_test1"}, resp = {10, (BYTE *)"resp_test1"}, ns = {2, (BYTE *)"ns"};
    WS_CHANNEL *channel;
    WS_MESSAGE *msg;
    WS_ELEMENT_DESCRIPTION body;
    WS_MESSAGE_DESCRIPTION desc_req, desc_resp;
    const WS_MESSAGE_DESCRIPTION *desc[1];
    INT32 val = -1;
    HRESULT hr;

    hr = create_channel( port, &channel );
    ok( hr == S_OK, "got %08x\n", hr );

    hr = WsCreateMessageForChannel( channel, NULL, 0, &msg, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    set_elem_desc( &body, &req, &ns, WS_INT32_TYPE, NULL );
    set_msg_desc( &desc_req, &req, &body );
    hr = WsSendMessage( channel, msg, &desc_req, WS_WRITE_REQUIRED_VALUE, &val, sizeof(val), NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );
    WsFreeMessage( msg );

    hr = WsCreateMessageForChannel( channel, NULL, 0, &msg, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    set_elem_desc( &body, &resp, &ns, WS_INT32_TYPE, NULL );
    set_msg_desc( &desc_resp, &resp, &body );
    desc[0] = &desc_resp;
    hr = WsReceiveMessage( NULL, msg, desc, 1, WS_RECEIVE_REQUIRED_MESSAGE, WS_READ_REQUIRED_VALUE,
                           NULL, &val, sizeof(val), NULL, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    hr = WsReceiveMessage( channel, NULL, desc, 1, WS_RECEIVE_REQUIRED_MESSAGE, WS_READ_REQUIRED_VALUE,
                           NULL, &val, sizeof(val), NULL, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    hr = WsReceiveMessage( channel, msg, NULL, 1, WS_RECEIVE_REQUIRED_MESSAGE, WS_READ_REQUIRED_VALUE,
                           NULL, &val, sizeof(val), NULL, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    hr = WsReceiveMessage( channel, msg, desc, 1, WS_RECEIVE_REQUIRED_MESSAGE, WS_READ_REQUIRED_VALUE,
                           NULL, &val, sizeof(val), NULL, NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );
    ok( val == -2, "got %d\n", val );

    hr = WsCloseChannel( channel, NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    WsFreeChannel( channel );
    WsFreeMessage( msg );
}

static HRESULT create_proxy( int port, WS_SERVICE_PROXY **ret )
{
    static const WCHAR fmt[] =
        {'h','t','t','p',':','/','/','1','2','7','.','0','.','0','.','1',':','%','u','/',0};
    WS_ENVELOPE_VERSION env_version;
    WS_ADDRESSING_VERSION addr_version;
    WS_CHANNEL_PROPERTY prop[2];
    WS_ENDPOINT_ADDRESS addr;
    WS_SERVICE_PROXY *proxy;
    WCHAR url[64];
    HRESULT hr;

    env_version = WS_ENVELOPE_VERSION_SOAP_1_1;
    prop[0].id        = WS_CHANNEL_PROPERTY_ENVELOPE_VERSION;
    prop[0].value     = &env_version;
    prop[0].valueSize = sizeof(env_version);

    addr_version = WS_ADDRESSING_VERSION_TRANSPORT;
    prop[1].id        = WS_CHANNEL_PROPERTY_ADDRESSING_VERSION;
    prop[1].value     = &addr_version;
    prop[1].valueSize = sizeof(addr_version);

    *ret = NULL;
    hr = WsCreateServiceProxy( WS_CHANNEL_TYPE_REQUEST, WS_HTTP_CHANNEL_BINDING, NULL, NULL,
                               0, prop, sizeof(prop)/sizeof(prop[0]), &proxy, NULL );
    if (hr != S_OK) return hr;

    addr.url.length = wsprintfW( url, fmt, port );
    addr.url.chars  = url;
    addr.headers    = NULL;
    addr.extensions = NULL;
    addr.identity   = NULL;
    hr = WsOpenServiceProxy( proxy, &addr, NULL, NULL );
    if (hr == S_OK) *ret = proxy;
    else WsFreeServiceProxy( proxy );
    return hr;
}

static const char req_test2[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>"
    "<req_test2 xmlns=\"ns\"><val>1</val><str>test</str><str>test2</str></req_test2>"
    "</s:Body></s:Envelope>";

static const char resp_test2[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>"
    "<resp_test2 xmlns=\"ns\"><str>test</str><val>1</val><val>2</val></resp_test2>"
    "</s:Body></s:Envelope>";

static void test_WsCall( int port )
{
    static const WCHAR testW[] = {'t','e','s','t',0}, test2W[] = {'t','e','s','t','2',0};
    WS_XML_STRING str = {3, (BYTE *)"str"};
    WS_XML_STRING req = {3, (BYTE *)"req"};
    WS_XML_STRING resp = {4, (BYTE *)"resp"};
    WS_XML_STRING req_elem = {9, (BYTE *)"req_test2"};
    WS_XML_STRING resp_elem = {10, (BYTE *)"resp_test2"};
    WS_XML_STRING req_action = {9, (BYTE *)"req_test2"};
    WS_XML_STRING resp_action = {10, (BYTE *)"resp_test2"};
    WS_XML_STRING val = {3, (BYTE *)"val"};
    WS_XML_STRING ns = {2, (BYTE *)"ns"};
    HRESULT hr;
    WS_SERVICE_PROXY *proxy;
    WS_OPERATION_DESCRIPTION op;
    WS_MESSAGE_DESCRIPTION input_msg, output_msg;
    WS_ELEMENT_DESCRIPTION input_elem, output_elem;
    WS_STRUCT_DESCRIPTION input_struct, output_struct;
    WS_FIELD_DESCRIPTION f, f2, f3, f4, *fields[2], *fields2[2];
    WS_PARAMETER_DESCRIPTION param[6];
    const void *args[6];
    WS_HEAP *heap;
    INT32 **val_ptr;
    WCHAR **str_ptr;
    ULONG *count_ptr;
    const WCHAR *str_array[2];
    struct input
    {
        INT32         val;
        ULONG         count;
        const WCHAR **str;
    } in;
    struct output
    {
        WCHAR *str;
        ULONG  count;
        INT32 *val;
    } out;

    hr = WsCreateHeap( 1 << 16, 0, NULL, 0, &heap, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    hr = WsCall( NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    hr = create_proxy( port, &proxy );
    ok( hr == S_OK, "got %08x\n", hr );

    hr = WsCall( proxy, NULL, NULL, NULL, NULL, 0, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    set_field_desc( &f, WS_ELEMENT_FIELD_MAPPING, &val, &ns, WS_INT32_TYPE, NULL, 0, 0, 0, NULL, NULL );
    set_field_desc( &f4, WS_REPEATING_ELEMENT_FIELD_MAPPING, NULL, NULL, WS_WSZ_TYPE, NULL,
                    FIELD_OFFSET(struct input, str), 0, FIELD_OFFSET(struct input, count), &str, &ns );
    fields[0] = &f;
    fields[1] = &f4;

    set_struct_desc( &input_struct, sizeof(struct input), TYPE_ALIGNMENT(struct input), fields, 2, &req, &ns, 0 );
    set_elem_desc( &input_elem, &req_elem, &ns, WS_STRUCT_TYPE, &input_struct );
    set_msg_desc( &input_msg, &req_action, &input_elem );

    set_field_desc( &f2, WS_ELEMENT_FIELD_MAPPING, &str, &ns, WS_WSZ_TYPE, NULL, FIELD_OFFSET(struct output, str),
                    0, 0, NULL, NULL );
    set_field_desc( &f3, WS_REPEATING_ELEMENT_FIELD_MAPPING, NULL, NULL, WS_INT32_TYPE, NULL,
                    FIELD_OFFSET(struct output, val), 0, FIELD_OFFSET(struct output, count), &val, &ns );
    fields2[0] = &f2;
    fields2[1] = &f3;

    set_struct_desc( &output_struct, sizeof(struct output), TYPE_ALIGNMENT(struct output), fields2, 2, &resp, &ns, 0 );
    set_elem_desc( &output_elem, &resp_elem, &ns, WS_STRUCT_TYPE, &output_struct );
    set_msg_desc( &output_msg, &resp_action, &output_elem );

    set_param_desc( &param[0], WS_PARAMETER_TYPE_NORMAL, 0, 0xffff );
    set_param_desc( &param[1], WS_PARAMETER_TYPE_ARRAY, 1, 0xffff );
    set_param_desc( &param[2], WS_PARAMETER_TYPE_ARRAY_COUNT, 1, 0xffff );
    set_param_desc( &param[3], WS_PARAMETER_TYPE_NORMAL, 0xffff, 0 );
    set_param_desc( &param[4], WS_PARAMETER_TYPE_ARRAY, 0xffff, 1 );
    set_param_desc( &param[5], WS_PARAMETER_TYPE_ARRAY_COUNT, 0xffff, 1 );

    set_op_desc( &op, &input_msg, &output_msg, 6, param );
    hr = WsCall( proxy, &op, NULL, NULL, NULL, 0, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    in.val   = 1;
    str_array[0] = testW;
    str_array[1] = test2W;
    in.str   = str_array;
    in.count = 2;

    args[0] = &in.val;
    args[1] = &in.str;
    args[2] = &in.count;

    out.str   = NULL;
    out.count = 0;
    out.val   = NULL;
    str_ptr   = &out.str;
    val_ptr   = &out.val;
    count_ptr = &out.count;

    args[3] = &str_ptr;
    args[4] = &val_ptr;
    args[5] = &count_ptr;

    hr = WsCall( proxy, &op, args, heap, NULL, 0, NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );
    ok( !lstrcmpW( out.str, testW ), "wrong data\n" );
    ok( out.count == 2, "got %u\n", out.count );
    ok( out.val[0] == 1, "got %u\n", out.val[0] );
    ok( out.val[1] == 2, "got %u\n", out.val[1] );

    hr = WsCloseServiceProxy( proxy, NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    WsFreeServiceProxy( proxy );
    WsFreeHeap( heap );
}

static const char req_test3[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>"
    "<req_test3 xmlns=\"ns\"><val>1</val></req_test3>"
    "</s:Body></s:Envelope>";

static const char resp_test3[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body/></s:Envelope>";

static void test_empty_response( int port )
{
    WS_XML_STRING req = {3, (BYTE *)"req"};
    WS_XML_STRING resp = {4, (BYTE *)"resp"};
    WS_XML_STRING req_action = {9, (BYTE *)"req_test3"};
    WS_XML_STRING resp_action = {10, (BYTE *)"resp_test3"};
    WS_XML_STRING req_elem = {9, (BYTE *)"req_test3"};
    WS_XML_STRING resp_elem = {10, (BYTE *)"resp_test3"};
    WS_XML_STRING ns = {2, (BYTE *)"ns"};
    WS_XML_STRING ns2 = {0, (BYTE *)""};
    WS_XML_STRING val = {3, (BYTE *)"val"};
    HRESULT hr;
    WS_SERVICE_PROXY *proxy;
    WS_FIELD_DESCRIPTION f, *fields[1];
    WS_STRUCT_DESCRIPTION input_struct, output_struct;
    WS_ELEMENT_DESCRIPTION input_elem, output_elem;
    WS_MESSAGE_DESCRIPTION input_msg, output_msg;
    WS_PARAMETER_DESCRIPTION param[1];
    WS_OPERATION_DESCRIPTION op;
    const void *args[1];
    WS_HEAP *heap;
    struct input
    {
        INT32 val;
    } in;

    hr = WsCreateHeap( 1 << 16, 0, NULL, 0, &heap, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    hr = create_proxy( port, &proxy );
    ok( hr == S_OK, "got %08x\n", hr );

    set_field_desc( &f, WS_ELEMENT_FIELD_MAPPING, &val, &ns, WS_INT32_TYPE, NULL, 0, 0, 0, NULL, NULL );
    fields[0] = &f;

    set_struct_desc( &input_struct, sizeof(struct input), TYPE_ALIGNMENT(struct input), fields, 1, &req, &ns, 0 );
    set_elem_desc( &input_elem, &req_elem, &ns, WS_STRUCT_TYPE, &input_struct );
    set_msg_desc( &input_msg, &req_action, &input_elem );

    set_struct_desc( &output_struct, 0, 1, NULL, 0, &resp, &ns2, 0x6 );
    set_elem_desc( &output_elem, &resp_elem, &ns, WS_STRUCT_TYPE, NULL );
    set_msg_desc( &output_msg, &resp_action, &output_elem );

    set_param_desc( param, WS_PARAMETER_TYPE_NORMAL, 0, 0xffff );
    set_op_desc( &op, &input_msg, &output_msg, 1, param );

    in.val = 1;
    args[0] = &in.val;
    hr = WsCall( proxy, &op, args, heap, NULL, 0, NULL, NULL );
    ok( hr == E_INVALIDARG, "got %08x\n", hr );

    set_elem_desc( &output_elem, &resp_elem, &ns, WS_STRUCT_TYPE, &output_struct );
    hr = WsCall( proxy, &op, args, heap, NULL, 0, NULL, NULL );
    ok( hr == WS_E_INVALID_FORMAT, "got %08x\n", hr );

    hr = WsCloseServiceProxy( proxy, NULL, NULL );
    ok( hr == S_OK, "got %08x\n", hr );

    WsFreeServiceProxy( proxy );
    WsFreeHeap( heap );
}

static const char status_200[] = "HTTP/1.1 200 OK\r\n";

static const struct
{
    const char   *req_action;
    const char   *req_data;
    unsigned int  req_len;
    const char   *resp_status;
    const char   *resp_data;
    unsigned int  resp_len;
}
tests[] =
{
    { "req_test1", req_test1, sizeof(req_test1)-1, status_200, resp_test1, sizeof(resp_test1)-1 },
    { "req_test2", req_test2, sizeof(req_test2)-1, status_200, resp_test2, sizeof(resp_test2)-1 },
    { "req_test3", req_test3, sizeof(req_test3)-1, status_200, resp_test3, sizeof(resp_test3)-1 },
};

static void send_response( int c, const char *status, const char *data, unsigned int len )
{
    static const char headers[] =
        "Content-Type: text/xml; charset=utf-8\r\nConnection: close\r\n";
    static const char fmt[] =
        "Content-Length: %u\r\n\r\n";
    char buf[128];

    send( c, status, strlen(status), 0 );
    send( c, headers, sizeof(headers) - 1, 0 );
    sprintf( buf, fmt, len );
    send( c, buf, strlen(buf), 0 );
    send( c, data, len, 0 );
}

struct server_info
{
    HANDLE event;
    int    port;
};

static DWORD CALLBACK server_proc( void *arg )
{
    struct server_info *info = arg;
    int len, res, c = -1, i, j, on = 1, quit;
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in sa;
    char buf[1024];
    const char *p;

    WSAStartup( MAKEWORD(1,1), &wsa );
    if ((s = socket( AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET) return 1;
    setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on) );

    memset( &sa, 0, sizeof(sa) );
    sa.sin_family           = AF_INET;
    sa.sin_port             = htons( info->port );
    sa.sin_addr.S_un.S_addr = inet_addr( "127.0.0.1" );
    if (bind( s, (struct sockaddr *)&sa, sizeof(sa) ) < 0) return 1;

    listen( s, 0 );
    SetEvent( info->event );
    for (;;)
    {
        c = accept( s, NULL, NULL );

        buf[0] = 0;
        for (i = 0; i < sizeof(buf) - 1; i++)
        {
            if ((res = recv( c, &buf[i], 1, 0 )) != 1) break;
            if (i < 4) continue;
            if (buf[i - 2] == '\n' && buf[i] == '\n' && buf[i - 3] == '\r' && buf[i - 1] == '\r')
                break;
        }
        buf[i] = 0;
        quit = strstr( buf, "SOAPAction: \"quit\"" ) != NULL;

        len = 0;
        if ((p = strstr( buf, "Content-Length: " )))
        {
            p += strlen( "Content-Length: " );
            while (isdigit( *p ))
            {
                len *= 10;
                len += *p++ - '0';
            }
        }
        for (i = 0; i < len; i++)
        {
            if ((res = recv( c, &buf[i], 1, 0 )) != 1) break;
        }
        buf[i] = 0;

        for (j = 0; j < sizeof(tests)/sizeof(tests[0]); j++)
        {
            if (strstr( buf, tests[j].req_action ))
            {
                if (tests[j].req_data)
                {
                    int data_len = strlen( buf );
                    ok( tests[j].req_len == data_len, "%u: unexpected data length %u %u\n",
                        j, data_len, tests[j].req_len );
                    if (tests[j].req_len == data_len)
                        ok( !memcmp( tests[j].req_data, buf, tests[j].req_len ), "%u: unexpected data %s\n", j, buf );
                }
                send_response( c, tests[j].resp_status, tests[j].resp_data, tests[j].resp_len );
                break;
            }
        }

        shutdown( c, 2 );
        closesocket( c );
        if (quit) break;
    }

    return 0;
}

START_TEST(proxy)
{
    WS_XML_STRING test1 = {9, (BYTE *)"req_test1"};
    WS_XML_STRING quit = {4, (BYTE *)"quit"};
    struct server_info info;
    HANDLE thread;
    DWORD ret;

    test_WsCreateServiceProxy();
    test_WsCreateServiceProxyFromTemplate();
    test_WsOpenServiceProxy();

    info.port  = 7533;
    info.event = CreateEventW( NULL, 0, 0, NULL );
    thread = CreateThread( NULL, 0, server_proc, &info, 0, NULL );
    ok( thread != NULL, "failed to create server thread %u\n", GetLastError() );

    ret = WaitForSingleObject( info.event, 3000 );
    ok(ret == WAIT_OBJECT_0, "failed to start test server %u\n", GetLastError());
    if (ret != WAIT_OBJECT_0) return;

    test_WsSendMessage( info.port, &test1 );
    test_WsReceiveMessage( info.port );
    test_WsCall( info.port );
    test_empty_response( info.port );

    test_WsSendMessage( info.port, &quit );
    WaitForSingleObject( thread, 3000 );
}
