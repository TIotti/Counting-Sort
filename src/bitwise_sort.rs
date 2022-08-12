use std::collections::HashMap;
use std::hash::Hash;

pub struct BitwiseElement<T> {
    pub group: u8,
    pub data: *const T
}

pub struct BitwiseSort<T> {
    pub elements: HashMap<String, BitwiseElement<T>>,
    pub groups: HashMap<u8, HashMap<String, *const T>>
}
fn add_element<T>(bitwise_sort: &mut BitwiseSort<T>, key:&str, element:&T, is_list_a: bool){
    match bitwise_sort.elements.insert(key.to_string(), BitwiseElement{group: if is_list_a {1} else {2}, data: element}){
        None => {
            bitwise_sort.groups.entry(if is_list_a {1} else {2}).or_insert(HashMap::new()).insert((*key).to_string(), element);
        },
        Some(mut data) => {

            if data.group != 0 {      
                if data.group == 3 {return;}
                bitwise_sort.groups.entry(data.group).or_insert(HashMap::new()).remove(key);
                bitwise_sort.groups.entry(3).or_insert(HashMap::new()).insert((*key).to_string(), element);        
            }
            data.group |= if is_list_a {1} else {2};
        }
    };
    
}
pub fn insert_element_a<T>(bitwise_sort: &mut BitwiseSort<T>, key:String, element:&T){
    add_element(bitwise_sort, &key[..], element, true);
}

pub fn insert_element_b<T>(bitwise_sort: &mut BitwiseSort<T>, key:String, element:&T){
    add_element(bitwise_sort, &key[..], element, false);
}

