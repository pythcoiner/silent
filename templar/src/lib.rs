#[cxx::bridge]
mod ffi {
    extern "Rust" {
        type Account;
    }
}

pub struct Account {
    _placeholder: (),
}
