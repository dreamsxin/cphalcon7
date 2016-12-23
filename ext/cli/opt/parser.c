
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include  <stdlib.h>
#include  <string.h>
#include  <ctype.h>
#include  <assert.h>

#include "php.h"
#include  "cli/opt/parser.h"

struct option
{
    char const* name;
    char short_name;
    char const* txt;
    char const* group_name;
    int exist;
    char const* possible_arg;
    char const* help;
    char* buff;
    void (*help_printer)(void*);
    void* context;
    phalcon_option_value value;
    struct option* next;
    struct option* prev;
};


void display_help(FILE* file, struct option* o)
{
    static char buf[80];
    if ( o->group_name )
    {
        fprintf(file, "\n%s:\n", o->group_name);
        return ;
    }
    if ( o->txt )
    {
        fprintf(file, "\n%s\n", o->txt);
        return ;
    }
    int n = 0;
    if ( o->short_name )
    {
        n += snprintf(buf + n, sizeof buf - n, "-%c ", o->short_name);
    }
    if ( o->short_name && strlen(o->name) > 1 )
    {
        n += snprintf(buf + n, sizeof buf - n,"[ --%s ] ", o->name);
    }
    if ( !o->short_name && strlen(o->name) > 1 )
    {
        n += snprintf(buf + n, sizeof buf - n, "--%s ", o->name);
    }

    if ( o->value )
    {
        n += snprintf(buf + n , sizeof buf - n, "arg ");
        if ( o->value->has_default )
        {
            n += snprintf(buf + n , sizeof buf - n, "(=%s) ",
                    o->value->ops->display_default?o->value->ops->display_default(o->value):"obj");
        }
    }

    n = fprintf(file, "  %-40s", buf);
    if ( n > 42 )
    {
        fprintf(file, "\n  %40s", " ");
    }
    fprintf(file, "%s\n", o->help?o->help:"");
}


phalcon_option_cmd_chain option_add(char const* option_name, char const* help, void* pvalue);
phalcon_option_cmd_chain option_group(char const* group_name);
phalcon_option_cmd_chain option_text(char const* txt);
phalcon_option_cmd_chain option_more_help(char const* option_name, char const* help, void* pvalue, void(*printer)(void*), void* context);
phalcon_option_cmd_chain option_help(char const* help);
void option_parse_into(int argc, char const * const*argv, phalcon_optparser* parser);
static struct option* g_options = NULL;

static struct option* next_option(struct option* o)
{
    do{
        o = o->next;
    }while(!o->name);
    return o;
}

#define foreach_option( var, head ) for(struct option* var = ((struct option*)(head))->next; var != head; var = var->next)
#define foreach_argument_option( var, head ) for(struct option* var = next_option(((struct option*)(head))); var != head; var = next_option(var))
struct _phalcon_option_cmd_chain g_cmd_chain = {
    .add = &option_add,
    .parse_into = &option_parse_into,
    .text = &option_text,
    .group = &option_group,
    .more_help = &option_more_help,
    .help = &option_help,
};
phalcon_option_cmd_chain phalcon_opt_init(char const* desc)
{
    g_options = (struct option*)malloc(sizeof *g_options);
    bzero(g_options, sizeof *g_options);
    g_options->next = g_options->prev = g_options;
    g_options->name = desc;
    return &g_cmd_chain;
}

phalcon_option_cmd_chain option_more_help(char const* option_name, char const* help, void* pvalue, void(*printer)(void*), void* context)
{
    option_add(option_name, help, pvalue);
    struct option* o = g_options->prev;
    o->help_printer = printer;
    o->context = context;
    return &g_cmd_chain;
}
static void insert_option(struct option* opt)
{
    opt->prev = g_options->prev;
    opt->next = g_options;
    g_options->prev->next = opt;
    g_options->prev = opt;
}
phalcon_option_cmd_chain option_help(char const* help)
{
    return option_add("help,h", help, NULL);
}

phalcon_option_cmd_chain option_text(char const* txt)
{
    assert(txt);
    struct option * opt = (struct option*)malloc(sizeof *opt);
    bzero(opt, sizeof *opt);
    opt->txt = txt;
    insert_option(opt);
    return &g_cmd_chain;
}

phalcon_option_cmd_chain option_group(char const* group_name)
{
    assert(group_name);
    struct option * opt = (struct option*)malloc(sizeof *opt);
    bzero(opt, sizeof *opt);
    opt->group_name = group_name;
    insert_option(opt);
    return &g_cmd_chain;
}

#define PRE(s) (strlen(s) == 1?"-":"--")
phalcon_option_cmd_chain option_add(char const* option_name, char const* help, void* pvalue)
{
    assert(option_name);
    struct option * opt = (struct option*)malloc(sizeof *opt);
    bzero(opt, sizeof *opt);
    if(strlen(option_name) == 1)
    {
        opt->name = option_name;
        opt->short_name = *option_name;
    }else
    {
        opt->buff = strdup(option_name);
        char *p = strchr(opt->buff, ',');
        if ( p )
        {
            *p = '\0';
            opt->short_name = *(p+1);
        }
        opt->name = opt->buff;
    }
    foreach_argument_option(p, g_options)
    {
        if ( (opt->short_name == p->short_name && opt->short_name != '\0')|| strcmp(opt->name, p->name) == 0 )
        {
            fprintf(stderr, "%s%s redefine option name or short name\n", PRE(opt->name),opt->name);
            exit(1);
        }
    }
    opt->help = help;
    if ( pvalue)
    {
        phalcon_option_value ov = *(phalcon_option_value*)(pvalue);
        *(phalcon_option_value*)pvalue = NULL;
        opt->value = ov;
    }
    insert_option(opt);
    return &g_cmd_chain;
}


static struct option* find_option(char sh)
{
    foreach_argument_option(p, g_options)
    {
        if ( sh == p->short_name )
            return p;
    }
    return NULL;
}
static struct option* find_option_long(char const* name)
{
    foreach_argument_option(p , g_options)
    {
        if ( strcmp(name, p->name)  == 0)
            return p;
    }
    return NULL;

}
void check_option_repeat_or_no_def(struct option* o, char const* arg)
{
    if ( o == NULL)
    {
        fprintf(stderr, "option %s not defined\n", arg);
        exit(1);
    }
    if ( o->exist )
    {
        fprintf(stderr, "Waring:option %s repeat\n", o->name);
    }
}
void check_option_repeat_or_no_def_short(struct option* o, char arg)
{
    if ( o == NULL)
    {
        fprintf(stderr, "option -%c not defined\n", arg);
        exit(1);
    }
    if ( o->exist )
    {
        fprintf(stderr, "Waring:option -%c repeat\n", arg);
    }
}

static int may_be_arg(const char* arg)
{
    if ( *arg != '-' )
        return 1;
    if (strlen(arg) > 2 && *arg == '-' && (isdigit(arg[1]) || arg[1] == '.'))
        return 1;
    return 0;
}

static int parse_cmd(int argc, char const * const* argv)
{
    int i = 1;
    while(i < argc)
    {
        if (may_be_arg(argv[i]))
            break;
        if ( strlen(argv[i]) == 2 && *argv[i] == '-' && *(argv[i]+1) != '-')
        {
            char short_name = *(argv[i]+1);
            struct option* o = find_option(short_name);
            check_option_repeat_or_no_def(o, argv[i]);
            o->exist = 1;
            if (o->value && i+1 < argc && may_be_arg(argv[i+1]) )
            {
                o->possible_arg = argv[i+1];
                ++i;
            }
            ++i;
        }else if ( strlen(argv[i]) > 2 && *argv[i] == '-' && *(argv[i]+1) != '-')
        {
            char const* last = argv[i] + strlen(argv[i]);
            struct option* o = NULL;
            for(char const* p = argv[i]+1; p < last; ++p)
            {
                o = find_option(*p);
                check_option_repeat_or_no_def_short(o,*p);
                o->exist = 1;
                if ( o->value )
                {
                    if ( p == last - 1 )
                    {
                        if (i + 1< argc && may_be_arg(argv[i+1]))
                        {
                            o->possible_arg = argv[i+1];
                            ++i;
                        }
                    }else
                    {
                        o->possible_arg = p+1;
                    }
                    break;
                }
            }
            ++i;
        }else if ( strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] != '-')
        {
            char * name = strdup(argv[i] + 2);
            char * p = strchr(name, '=');
            char const* possible_arg = NULL;
            if ( p  )
            {
                *p = '\0';
                possible_arg = argv[i] + 2 + (p - name) + 1;
            }
            struct option * o = find_option_long(name);
            free(name);
            check_option_repeat_or_no_def(o , argv[i]);
            o->exist = 1;
            if ( possible_arg && *possible_arg != '\0' )
            {
                o->possible_arg = possible_arg;
            }else if ( i +1 < argc && may_be_arg(argv[i+1]) )
            {
                o->possible_arg = argv[i+1];
                ++i;
            }
            ++i;
        }else{
            fprintf(stderr, "illegal format option %s\n", argv[i]);
            exit(1);
        }
    }
    return i;
}
static void notify_output(int for_helper)
{
    foreach_argument_option(o, g_options)
    {
        if ( for_helper && !o->help_printer)  continue;
        if ( !for_helper && o->help_printer ) continue;
        if ( ! o->value ) continue;
        if ( o->exist && o->possible_arg )
        {
            if(o->value->ops->validate)
            {
                char const* err_msg = o->value->ops->validate(o->value, o->possible_arg);
                if ( err_msg )
                {
                    fprintf(stderr, "option %s%s argument illegal: %s\n", PRE(o->name),o->name, err_msg);
                    exit(1);
                }
            }
            if ( o->value->pointer )
            {
                char const* err_msg = o->value->ops->parse(o->value, o->possible_arg);
                if ( err_msg )
                {
                    fprintf(stderr, "option %s%s argument illegal: %s\n", PRE(o->name),o->name, err_msg);
                    exit(1);
                }
                continue;
            }
        }

        if ( o->value->required )
        {
            if ( !o->exist && !o->value->has_default)
            {
                fprintf(stderr, "option %s%s required but missing\n", PRE(o->name),o->name);
                exit(1);
            }
            if ( o->exist && o->value->pointer && o->value->has_default)
            {
                o->value->ops->store_default(o->value);
                continue;
            }
            if ( ! o->exist )
            {
                if ( o->value->pointer ) o->value->ops->store_default(o->value);
                continue;
            }
            if (o->exist && !o->value->has_default)
            {
                fprintf(stderr, "option %s%s need argument but missing\n", PRE(o->name),o->name);
                exit(1);
            }
        }else if ( o->exist )
        {
            if ( !o->value->has_default )
            {
                fprintf(stderr, "option %s%s need argument but missing\n", PRE(o->name),o->name);
                exit(1);
            }else if (o->value->pointer)
            {
                o->value->ops->store_default(o->value);
            }

        }
    }
}
void option_parse_into(int argc, char const * const* argv, phalcon_optparser* pparser)
{
    *pparser = (struct _phalcon_optparser*)malloc(sizeof(**pparser));
    (*pparser)->_private = g_options;

    int i = parse_cmd(argc, argv);

    if ( i == 1  && g_options->next != g_options)
    {
        phalcon_opt_print(*pparser);
        exit(0);
    }

    foreach_argument_option(o , g_options)
    {
        if ( strcmp("help", o->name) == 0 && o->exist)
        {
            phalcon_opt_print(*pparser);
            exit(0);
        }
    }

    const int for_helper = 1;
    notify_output(for_helper);
    foreach_argument_option(o , g_options)
    {
        if( o->exist && o->help_printer )
        {
            o->help_printer(o->context);
            exit(0);
        }
    }
    notify_output(!for_helper);

    (*pparser)->argc = argc - i;
    (*pparser)->argv = argv + i;
    g_options = NULL;
}


int phalcon_opt_has(phalcon_optparser parser, char const* key)
{
    foreach_argument_option(o, parser->_private)
    {
        if ( strcmp(key, o->name) == 0  )
        {
            if ( o->exist || (o->value && o->value->has_default) )
                return 1;
            break;
        }
    }
    return 0;
}
char const* phalcon_opt_get_arg(phalcon_optparser parser, char const* key)
{
    foreach_argument_option(o, parser->_private)
    {
        if ( o->name && strcmp(key, o->name)  == 0 )
        {
            if ( o->exist  || ( o->value && o->value->required ))
            {
                if (o->possible_arg)
                    return o->possible_arg;
                if (o->value && o->value->has_default)
                    return o->value->ops->display_default(o->value);
            }
            break;
        }
    }
    return NULL;
}

void phalcon_opt_fprint(FILE* file, phalcon_optparser parser)
{
    struct option* op = (struct option*)parser->_private;
    fprintf(file, "%s:\n", op->name);
    foreach_option(o, op)
    {
        display_help(file, o);
    }
}
void phalcon_opt_print(phalcon_optparser parser)
{
    phalcon_opt_fprint(stdout, parser);
}
static void free_option(struct option* o)
{
    if(o->buff)
        free(o->buff);
    if(o->value)
        o->value->ops->free(o->value);
    free(o);
}
void phalcon_opt_free(phalcon_optparser parser)
{
    struct option * p = (struct option*)parser->_private;
    p = p->next;
    while( p != parser->_private )
    {
        struct option* free_p = p;
        p = p->next;
        free_option(free_p);
    }
    free_option((struct option*)parser->_private);
    free(parser);
}
