use std::{fs::File, io::Read};

// fn file_not_found(file_name: &str) {
//    println!("File {file_name} was not found.\n");
// }

fn main() {
    let mut _open_res_ = File::open("../merry.merry");
    println!("Hello, world!");
}
