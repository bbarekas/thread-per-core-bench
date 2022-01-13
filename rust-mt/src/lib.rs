use std::sync::atomic::{AtomicU32, AtomicI32, Ordering};
use std::thread;
use std::sync::Arc;

struct ThreadSafeCounter {
    count: AtomicI32,
}

impl ThreadSafeCounter {
    fn increment(&self) { self.count.fetch_add(1, Ordering::SeqCst); }
}

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

    assert_eq!(s.load(Ordering::SeqCst), 10000000);
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
}

//
#[no_mangle]
pub extern "C" fn multi_thread_rs() {
    let a = AtomicU32::new(0);
    let b = AtomicU32::new(0);
    let c = AtomicU32::new(0);
    let d = AtomicU32::new(0);

    // Update 4 variables on different threads.
    let t1 = thread::spawn(move || {
        update(&a);
    });
    let t2 = thread::spawn(move || {
        update(&b);
    });
    let t3 = thread::spawn(move || {
        update(&c);
    });
    let t4 = thread::spawn(move || {
        update(&d);
    });

    t1.join().unwrap();
    t2.join().unwrap();
    t3.join().unwrap();
    t4.join().unwrap();

    //assert_eq!(a.load(Ordering::SeqCst), 10000000);
    //assert_eq!(b.load(Ordering::SeqCst), 10000000);
    //assert_eq!(c.load(Ordering::SeqCst), 10000000);
    //assert_eq!(d.load(Ordering::SeqCst), 10000000);

}

//
#[no_mangle]
pub extern "C" fn multi_thread_arc_rs() {
    let a = Arc::new(AtomicU32::new(0));
    let b = Arc::new(AtomicU32::new(0));
    let c = Arc::new(AtomicU32::new(0));
    let d = Arc::new(AtomicU32::new(0));


    // Update 4 variables on different threads.
    let t1 = thread::spawn(move || {
        update(&a);
        assert_eq!(a.load(Ordering::SeqCst), 10000000);
    });

    let t2 = thread::spawn(move || {
        update(&b);
        assert_eq!(b.load(Ordering::SeqCst), 10000000);
    });

    let t3 = thread::spawn(move || {
        update(&c);
        assert_eq!(c.load(Ordering::SeqCst), 10000000);
    });

    let t4 = thread::spawn(move || {
        update(&d);
        assert_eq!(d.load(Ordering::SeqCst), 10000000);
    });

    t1.join().unwrap();
    t2.join().unwrap();
    t3.join().unwrap();
    t4.join().unwrap();

    //assert_eq!(a.load(Ordering::SeqCst), 10000000);
    //assert_eq!(b.load(Ordering::SeqCst), 10000000);
    //assert_eq!(c.load(Ordering::SeqCst), 10000000);
    //assert_eq!(d.load(Ordering::SeqCst), 10000000);

}

//
#[no_mangle]
pub extern "C" fn thread_safe_rs(n: i32) {
    //
    let counter = Arc::new(ThreadSafeCounter { count: AtomicI32::new(0) });
    let mut threads = Vec::new();
    for _ in 0..n {
        let counter1 = Arc::clone(&counter);

        threads.push(thread::spawn(move || {
            counter1.increment();
        }));
    }
    for thread in threads { thread.join(); }
    //println!("{}", counter.count.load(Ordering::SeqCst));
}
