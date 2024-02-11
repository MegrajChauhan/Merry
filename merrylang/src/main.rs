use std::fs::File;
use std::env; // for command line arguments

// fn file_not_found(file_name: &str) {
//    println!("File {file_name} was not found.\n");
// }



fn main() {
    let mut _command_line_args_: Vec<String> = env::args().collect(); // get the command line argument
    
    
    // let mut _open_res_ = File::open("../merry.merry");
    println!("Hello, world!");
}
