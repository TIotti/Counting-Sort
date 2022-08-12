
mod bitwise_sort;

use std::error::Error;
use std::io;
use bitwise_sort::BitwiseSort;
use csv;
use serde::Deserialize;
use std::collections::HashMap;
use rayon::prelude::*;

#[derive(Debug, Deserialize)]
struct RecordLine {
    id: String,
    email: String,
}

fn read_csv(path: &str, list: &mut Vec<RecordLine>) -> Result<(), Box<dyn Error>> {
    let mut rdr = csv::Reader::from_path(path)?;
    
    let headers = rdr.headers()?;
    println!("{:?}", headers);
    for result in rdr.deserialize() {
        let record :RecordLine = result?;              
        list.push(record);
    }
    Ok(())
}

fn insert_sorter(sorter:&mut BitwiseSort<String>, source: &Vec<RecordLine>, is_list_a:bool)
{
    for last in source {
        if is_list_a 
        {
            bitwise_sort::insert_element_a(sorter, last.id.to_string(), &last.email);
        }
        else
        {
            bitwise_sort::insert_element_b(sorter, last.id.to_string(), &last.email);
        }
    }
}

fn insert_sorter_lists(sorter:&mut BitwiseSort<String>, list_a: &Vec<RecordLine>, list_b: &Vec<RecordLine>){
    insert_sorter(sorter, list_a, true);
    insert_sorter(sorter, list_b, false);
}

fn main()
{
    let mut sorter = BitwiseSort::<String>{ 
        elements: HashMap::new(), 
        groups: HashMap::new()
    };
    use std::time::Instant;

    println!("Reading CSV files");
    let now = Instant::now();
    let mut list_a = Vec::new();
    read_csv("list_a.csv", &mut list_a).unwrap();
    let mut list_b = Vec::new();
    read_csv("list_b.csv", &mut list_b).unwrap();
    let elapsed = now.elapsed();
    println!("List A: {} List B: {} in {:.2?}", list_a.len(), list_b.len(), elapsed);

    println!("Feeding lists to the sorter");
    let now1 = Instant::now();
    insert_sorter_lists(&mut sorter, &list_a, &list_b);
    let elapsed1 = now1.elapsed();
    println!("Total: {} in {:.2?}", sorter.elements.len(), elapsed1);

    println!("Collecting results");
    let now2 = Instant::now();
    let g1 = sorter.groups[&1].len();
    let g2 = sorter.groups[&2].len();
    let g3 = sorter.groups[&3].len();
    let elapsed2 = now2.elapsed();
    println!("Group A: {}, Group B: {}, Both: {} in {:.2?}", g1, g2, g3, elapsed2);    

}