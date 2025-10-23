



#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>



typedef struct {
	ngx_flag_t enable;//是否启用
} ngx_http_prefix_filter_conf_t;//配置结构体


typedef struct {
	ngx_int_t add_prefix;//是否添加前缀
} ngx_http_prefix_filter_ctx_t;//上下文结构体

static ngx_int_t ngx_http_prefix_filter_init(ngx_conf_t *cf);//初始化函数
static ngx_int_t ngx_http_prefix_filter_header_filter(ngx_http_request_t *r);//头部过滤器
static ngx_int_t ngx_http_prefix_filter_body_filter(ngx_http_request_t *r, ngx_chain_t *in);//主体过滤器

static ngx_str_t filter_prefix = ngx_string("<h2>Author : King</h2><p><a href=\"http://www.0voice.com\">0voice</a></p>");//添加的前缀

//模块结构体
static void *ngx_http_prefix_filter_create_conf(ngx_conf_t *cf) {

	ngx_http_prefix_filter_conf_t *conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_prefix_filter_conf_t));
	if (conf == NULL) {
		return NULL;
	}

	conf->enable = NGX_CONF_UNSET;

	return conf;
}

//合并配置
static char *ngx_http_prefix_filter_merge_conf(ngx_conf_t *cf, void *parent, void *child) {
	ngx_http_prefix_filter_conf_t *prev = (ngx_http_prefix_filter_conf_t*)parent;
	ngx_http_prefix_filter_conf_t *conf = (ngx_http_prefix_filter_conf_t*)child;

	ngx_conf_merge_value(conf->enable, prev->enable, 0);

	return NGX_CONF_OK;
}

static ngx_command_t ngx_http_prefix_filter_commands[] = {
	{
		ngx_string("add_prefix"),//指令名称
		NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_CONF_FLAG,//指令位置
		ngx_conf_set_flag_slot,	//指令处理函数
		NGX_HTTP_LOC_CONF_OFFSET,	//指令存储位置
		offsetof(ngx_http_prefix_filter_conf_t, enable),	//指令存储位置
		NULL	
	}, 
	ngx_null_command	//结束标志
};

//模块上下文   
static ngx_http_module_t ngx_http_prefix_filter_module_ctx = {
	NULL,	//conf
	ngx_http_prefix_filter_init,	//conf
	NULL,	//main
	NULL,	//main
	NULL,	//server
	NULL,	//server
	ngx_http_prefix_filter_create_conf,	//local
	ngx_http_prefix_filter_merge_conf	/local
};

//模块入口
ngx_module_t ngx_http_prefix_filter_module = {
	NGX_MODULE_V1,	//模块版本
	&ngx_http_prefix_filter_module_ctx,		//模块上下文
	ngx_http_prefix_filter_commands,	//模块指令
	NGX_HTTP_MODULE,	//模块类型
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING	//预留
}; 

static ngx_http_output_header_filter_pt ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt ngx_http_next_body_filter;



static ngx_int_t ngx_http_prefix_filter_init(ngx_conf_t *cf) {

	ngx_http_next_header_filter = ngx_http_top_header_filter;
	ngx_http_top_header_filter = ngx_http_prefix_filter_header_filter;

	ngx_http_next_body_filter = ngx_http_top_body_filter;
	ngx_http_top_body_filter = ngx_http_prefix_filter_body_filter;

	return NGX_OK;
}

static ngx_int_t ngx_http_prefix_filter_header_filter(ngx_http_request_t *r) {

	ngx_http_prefix_filter_ctx_t *ctx;
	ngx_http_prefix_filter_conf_t *conf;

	if (r->headers_out.status != NGX_HTTP_OK) {
		return ngx_http_next_header_filter(r);
	}

	ctx = ngx_http_get_module_ctx(r, ngx_http_prefix_filter_module);
	if (ctx) {
		return ngx_http_next_header_filter(r);
	}

	conf = ngx_http_get_module_loc_conf(r, ngx_http_prefix_filter_module);
	if (conf == NULL) {
		return ngx_http_next_header_filter(r);
	}
	if (conf->enable == 0) {
		return ngx_http_next_header_filter(r);
	}


	ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_prefix_filter_ctx_t));
	if (ctx == NULL) {
		return NGX_ERROR;
	}
	ctx->add_prefix = 0;

	ngx_http_set_ctx(r, ctx, ngx_http_prefix_filter_module);

	if (r->headers_out.content_type.len >= sizeof("text/html") - 1
		&& ngx_strncasecmp(r->headers_out.content_type.data, (u_char*)"text/html", sizeof("text/html")-1) == 0) {

		ctx->add_prefix = 1;
		if (r->headers_out.content_length_n > 0) {
			r->headers_out.content_length_n += filter_prefix.len;
		}

		
	}

	return ngx_http_prefix_filter_header_filter(r);
}

static ngx_int_t ngx_http_prefix_filter_body_filter(ngx_http_request_t *r, ngx_chain_t *in) {
	
	ngx_http_prefix_filter_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_prefix_filter_module);
	if (ctx == NULL || ctx->add_prefix != 1) {
		return ngx_http_next_body_filter(r, in);
	}
	
	ctx->add_prefix = 2;

	ngx_buf_t *b = ngx_create_temp_buf(r->pool, filter_prefix.len);
	b->start = b->pos = filter_prefix.data;
	b->last = b->pos + filter_prefix.len;

	ngx_chain_t *cl = ngx_alloc_chain_link(r->pool);
	cl->buf = b;
	cl->next = in;

	return ngx_http_next_body_filter(r, cl);
}








