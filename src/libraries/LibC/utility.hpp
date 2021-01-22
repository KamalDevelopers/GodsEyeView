template<typename T>
void swap(T& first, T& second)
{
    T temp = first;
    first = second;
    second = temp;
}
