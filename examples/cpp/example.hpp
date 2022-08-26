#ifndef LIBCZH_EXAMPLE_HPP
#define LIBCZH_EXAMPLE_HPP

#include <vector>

class Container
{
public:
  using value_type = int;
public:
  Container() = default;
  
  std::vector<int> c;
  
  auto end() { return c.end(); }
  
  auto insert(typename std::vector<int>::iterator it, int i) { return c.insert(it, i); }
};

class Range
{
public:
  using value_type = int;
public:
  class Iterator
  {
  private:
    int i;
  public:
    explicit Iterator(int position = 0) : i{position} {}
    
    int operator*() const { return i; }
    
    Iterator &operator++()
    {
      ++i;
      return *this;
    }
    
    bool operator!=(const Iterator &other) const { return i != other.i; }
  };

private:
  int a;
  int b;
public:
  Range(int from, int to)
      : a{from}, b{to} {}
  
  Iterator begin() const { return Iterator{a}; }
  
  Iterator end() const { return Iterator{b}; }
};

#endif
