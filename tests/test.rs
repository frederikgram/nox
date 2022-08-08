// The file extension is '.rs' purely for VSCODE syntax highlighting
{
func int test(int a) {
    threshold: str = 18;
    if (a >= threshold) {
        return 1;
    } else {
        return 0;
    }
}


func int main() {

    res: int = test(5);
    res = 3;
    if(res != 1) {
        print("Error, didn't pass threshold limit");
    } else {
        print("Success, passed threshold limit");
    }

    mychar: char;
    mychar = 'g';

    // @TODO : parsing error, we haven't implemented & as a unary operator yet
    //res = &mychar;

    // @TODO : How do we handle pointer types like these?
    // res: * int = 5;  Note that this is a pointer to an int
    // @TODO : having comment slashes inside a comment slash messes things up
}


}