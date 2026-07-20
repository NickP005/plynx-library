class PlynxStackOnly
{
protected:
  PlynxStackOnly() {}
  ~PlynxStackOnly() {}

private:
  /// @brief Declared as private to prevent usage of dynamic memory
  void* operator new(size_t size);
  /// @brief Declared as private to prevent usage of dynamic memory
  void operator delete(void *p);
};

class PlynxNonCopyable
{
protected:
  PlynxNonCopyable(){}
  ~PlynxNonCopyable(){}

private:
  /// @brief Declared as private to prevent usage of copy constructor
  PlynxNonCopyable(const PlynxNonCopyable&);
  /// @brief Declared as private to prevent usage of assignment operator
  PlynxNonCopyable& operator=(const PlynxNonCopyable&);
};

template<typename T>
class PlynxSingleton
  : public PlynxNonCopyable
{
public:
  /** @brief Returns the instance of the singleton type
   When called for the first time, the singleton instance will be
   created. All subsequent calls will return a reference to the
   previously created instance.
   @return The singleton instance
   */
  static T* instance()
  {
    static T instance;
    return &instance;
  }
protected:
  PlynxSingleton() {}
  ~PlynxSingleton() {}
};

