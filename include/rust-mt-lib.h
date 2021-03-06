//
// Created by bbarekas on 6/26/2021.
//

#ifndef MULTI_THREAD_RUST_MT_LIB_H
#define MULTI_THREAD_RUST_MT_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

void print_hello();
void single_thread_rs();
void multi_thread_rs();
void multi_thread_arc_rs();
void thread_safe_rs(int n);

#ifdef __cplusplus
}
#endif

#endif //MULTI_THREAD_RUST_MT_LIB_H
