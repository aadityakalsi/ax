/*
For license details see ../../LICENSE
*/

/**
 * \file hashtable.c
 * \date Aug 21, 2017
 */

#include "defs.h"

#include "ax/hashtable.h"

#include <string.h>

static
ax_sz str_hash(void const* k)
{
    ax_sz h = 0;
    ax_sz prime = 31;
    ax_const_str s = (ax_const_str)k;

    while (*s) {
        h = h + (*s)*prime;
        ++s;
    }

    return h;
}

static
int str_eq(void const* k0, void const* k1)
{
    return strcmp(k0, k1) == 0;
}

static
void del_kv(void* k, void* v)
{
    (void)k;
    (void)v;
}

static
void basic(void)
{
    ax_ht_t ht;
    ax_ht_entry_t* e;
    int ins = 0;
    testThat(ax_ht_init_default(&ht, str_hash, str_eq, del_kv) == 0);
    e = ax_ht_find(&ht, "foo");
    testThat(e == AX_NULL);

    e = ax_ht_insert(&ht, "foo", "1", &ins);
    testThat(ins == 1);
    testThat(e != AX_NULL);
    testThat(strcmp("foo", (ax_const_str)e->key) == 0);
    testThat(strcmp("1", (ax_const_str)e->value) == 0);

    e = ax_ht_find(&ht, "foo");
    testThat(e != AX_NULL);
    testThat(strcmp("foo", (ax_const_str)e->key) == 0);
    testThat(strcmp("1", (ax_const_str)e->value) == 0);

    e = ax_ht_insert(&ht, "foo", "1", &ins);
    testThat(ins == 0);
    testThat(e != AX_NULL);
    testThat(strcmp("foo", (ax_const_str)e->key) == 0);
    testThat(strcmp("1", (ax_const_str)e->value) == 0);

    {
        e = ax_ht_insert(&ht, "aoo", "1", &ins);
        testThat(ins == 1);
        testThat(e != AX_NULL);
        testThat(strcmp("aoo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_insert(&ht, "boo", "1", &ins);
        testThat(ins == 1);
        testThat(e != AX_NULL);
        testThat(strcmp("boo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_insert(&ht, "coo", "1", &ins);
        testThat(ins == 1);
        testThat(e != AX_NULL);
        testThat(strcmp("coo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_insert(&ht, "doo", "1", &ins);
        testThat(ins == 1);
        testThat(e != AX_NULL);
        testThat(strcmp("doo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_insert(&ht, "eoo", "1", &ins);
        testThat(ins == 1);
        testThat(e != AX_NULL);
        testThat(strcmp("eoo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_insert(&ht, "goo", "1", &ins);
        testThat(ins == 1);
        testThat(e != AX_NULL);
        testThat(strcmp("goo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_insert(&ht, "hoo", "1", &ins);
        testThat(ins == 1);
        testThat(e != AX_NULL);
        testThat(strcmp("hoo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }

    {
        e = ax_ht_find(&ht, "aoo");
        testThat(e != AX_NULL);
        testThat(strcmp("aoo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_find(&ht, "boo");
        testThat(e != AX_NULL);
        testThat(strcmp("boo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_find(&ht, "coo");
        testThat(e != AX_NULL);
        testThat(strcmp("coo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_find(&ht, "doo");
        testThat(e != AX_NULL);
        testThat(strcmp("doo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_find(&ht, "eoo");
        testThat(e != AX_NULL);
        testThat(strcmp("eoo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_find(&ht, "foo");
        testThat(e != AX_NULL);
        testThat(strcmp("foo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_find(&ht, "goo");
        testThat(e != AX_NULL);
        testThat(strcmp("goo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }
    {
        e = ax_ht_find(&ht, "hoo");
        testThat(e != AX_NULL);
        testThat(strcmp("hoo", (ax_const_str)e->key) == 0);
        testThat(strcmp("1", (ax_const_str)e->value) == 0);        
    }

    testThat(ax_ht_destroy(&ht) == 0);
    (void)ins;
}

setupSuite(ht)
{
    addTest(basic);
}
