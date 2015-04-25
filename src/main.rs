fn can_make_change(denominations: &[i32], value: i32) -> bool {
    let max = (value + 1) as usize;

    let mut change = vec![false; max];
    change[0] = true;

    for val in 1 .. (value + 1) {
        for d in denominations {
            let idx = val - d;
            if idx >= 0 && change[idx as usize] {
                change[val as usize] = true;
            }
        }
    }

    change[value as usize]
}

fn main() {
    let denominations = vec![17, 10];
    let value = 117;
    let change_p = can_make_change(&denominations, value);

    if change_p {
        println!("can make change!");
    }
    else {
        println!("no change can be made...");
    }
}
