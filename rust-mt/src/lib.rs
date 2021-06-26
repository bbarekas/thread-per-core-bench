use std::sync::atomic::{AtomicU32, Ordering};

#[no_mangle]
pub extern "C" fn print_hello() {
    //println!("Hello from Rust world!");
    let _i = 10;
}

//
fn update(s: &AtomicU32) {
    for _n in 0..10000000 {
        s.fetch_add(1, Ordering::Relaxed);
    }
}

//
#[no_mangle]
pub extern "C" fn single_thread_rs() {
    let a = AtomicU32::new(0);
    let b = AtomicU32::new(0);
    let c = AtomicU32::new(0);
    let d = AtomicU32::new(0);

    // Update 4 variables sequentially
    update(&a);
    update(&b);
    update(&c);
    update(&d);

    assert_eq!(a.load(Ordering::SeqCst), 10000000);
    assert_eq!(b.load(Ordering::SeqCst), 10000000);
    assert_eq!(c.load(Ordering::SeqCst), 10000000);
    assert_eq!(d.load(Ordering::SeqCst), 10000000);

}
