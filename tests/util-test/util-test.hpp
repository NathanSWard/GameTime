#pragma once

void chain_test();
void mutex_test();
void rwlock_test();
void common_test();
void meta_test();
void rng_test();
void type_map_test();
void uuid_test();
void rc_test();

void util_test()
{
    chain_test();
    mutex_test();
    rwlock_test();
    common_test();
    meta_test();
    rng_test();
    type_map_test();
    uuid_test();
    rc_test();
}