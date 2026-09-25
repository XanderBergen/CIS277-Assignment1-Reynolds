template <typename T>
class Stack

void push(const T& value);
T pop();
T& top();
bool empty() const;
size_t size() const;