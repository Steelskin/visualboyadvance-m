#![no_std]

#[no_mangle]
pub extern fn add(lhs: u32, rhs: u32) -> u32 {
    lhs + rhs
}

#[panic_handler]
/// Panic handler, called on panic. Does nothing.
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}
