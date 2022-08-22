#ifndef LIBCZH_EXAMPLE_HPP
#define LIBCZH_EXAMPLE_HPP

#include <vector>

class EgContainer
{
public:
  using value_type = int;
public:
  EgContainer() = default;
  
  std::vector<int> c;
  
  auto end() { return c.end(); }
  
  auto insert(typename std::vector<int>::iterator it, int i) { return c.insert(it, i); }
};

class EgRange
{
public:
  using value_type = int;
public:
  class EgIterator
  {
  private:
    int i;
  public:
    explicit EgIterator(int position = 0) : i{position} {}
    
    int operator*() const { return i; }
    
    EgIterator &operator++()
    {
      ++i;
      return *this;
    }
    
    bool operator!=(const EgIterator &other) const { return i != other.i; }
  };

private:
  int a;
  int b;
public:
  EgRange(int from, int to)
      : a{from}, b{to} {}
  
  EgIterator begin() const { return EgIterator{a}; }
  
  EgIterator end() const { return EgIterator{b}; }
};

#endif
