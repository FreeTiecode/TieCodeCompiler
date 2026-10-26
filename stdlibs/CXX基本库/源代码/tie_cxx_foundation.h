#ifndef TIE_CXX_FOUNDATION_H
#define TIE_CXX_FOUNDATION_H

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

#define T_INT int32_t
#define T_LONG int64_t
#define T_FLOAT float
#define T_DOUBLE double
#define T_BYTE int8_t
#define T_CHAR int8_t
#define T_BOOL bool

#ifdef _WIN32
using T_STRING = std::wstring;
#define _TSTR(ch) L##ch
#define PRIMITIVE_TO_STR(value) std::to_wstring(value)
#define GET_STR_CHARS(str) ConvertWStringToChars(str).get()
#else
using T_STRING = std::string;
#define _TSTR(ch) ch
#define PRIMITIVE_TO_STR(value) std::to_string(value)
#define GET_STR_CHARS(str) str
#endif

namespace tie {
  class CTieObject : std::enable_shared_from_this<CTieObject> {
  public:
    virtual ~CTieObject() = default;
    virtual T_STRING ToString();
  protected:
    virtual T_STRING GetTieClassName();
  };

  template<typename T>
  class CTiePrimitiveWrapper : public CTieObject {
  public:
    CTiePrimitiveWrapper(const T& value): m_value_(value) {}
    CTiePrimitiveWrapper(T&& value): m_value_(std::move(value)) {}

    T GetValue() const {
      return m_value_;
    }
  protected:
    T m_value_;
  };

  class CTieString : public CTiePrimitiveWrapper<T_STRING> {
  public:
    CTieString(const T_STRING& value): CTiePrimitiveWrapper(value) {}
    CTieString(T_STRING&& value): CTiePrimitiveWrapper(std::move(value)) {}

    T_STRING ToString() override {
      return m_value_;
    }

    T_STRING GetTieClassName() override {
      return _TSTR("CTieString");
    }
  };

  class CTieBool : public CTiePrimitiveWrapper<T_BOOL> {
  public:
    CTieBool(const T_BOOL& value): CTiePrimitiveWrapper(value) {}
    CTieBool(T_BOOL&& value): CTiePrimitiveWrapper(std::move(value)) {}

    T_STRING ToString() override {
      return m_value_ ? _TSTR("真") : _TSTR("假");
    }

    T_STRING GetTieClassName() override {
      return _TSTR("CTieBool");
    }
  };

#define DEF_WRAPPER_NUMERIC(class_name, type) \
  class class_name : public CTiePrimitiveWrapper<type> { \
  public: \
    class_name(type value) : CTiePrimitiveWrapper(value) {} \
    T_STRING ToString() override { \
      return PRIMITIVE_TO_STR(m_value_); \
    } \
    T_STRING GetTieClassName() override { \
      return _TSTR(#class_name); \
    } \
  };
  DEF_WRAPPER_NUMERIC(CTieChar, T_CHAR)
  DEF_WRAPPER_NUMERIC(CTieInt, T_INT)
  DEF_WRAPPER_NUMERIC(CTieLong, T_LONG)
  DEF_WRAPPER_NUMERIC(CTieFloat, T_FLOAT)
  DEF_WRAPPER_NUMERIC(CTieDouble, T_DOUBLE)
  DEF_WRAPPER_NUMERIC(CTieByte, T_BYTE)

  template<typename E>
  class CTieArray : public CTieObject {
  public:
    CTieArray(const std::initializer_list<E>& elements): m_elements_(elements) {
    }

    CTieArray(std::initializer_list<E>&& elements): m_elements_(std::move(elements)) {
    }

    template<typename U>
    explicit CTieArray(T_INT capacity, U&& def_value) {
      m_elements_.resize(capacity, std::forward<U>(def_value));
    }

    T_INT GetLength() const {
      return m_elements_.size();
    }

    E& operator[](T_INT index) {
      return m_elements_[index];
    }

    E operator[](T_INT index) const {
      return m_elements_[index];
    }

    typename std::vector<E>::iterator begin() {
      return m_elements_.begin();
    }

    typename std::vector<E>::iterator end() {
      return m_elements_.end();
    }

    typename std::vector<E>::const_iterator begin() const {
      return m_elements_.begin();
    }

    typename std::vector<E>::const_iterator end() const {
      return m_elements_.end();
    }
  protected:
    T_STRING GetTieClassName() override {
      return _TSTR("CTieArray");
    }
  private:
    std::vector<E> m_elements_;
  };

  template<typename ContainerInstanceType, typename ElementType>
  class CTieIteratorBase : public CTieObject {
  public:
    CTieIteratorBase(const typename ContainerInstanceType::iterator& begin,
                       const typename ContainerInstanceType::iterator& end): m_iterator_(begin), m_iterator_end_(end) {}

    virtual bool HasNext() const {
      return m_iterator_ != m_iterator_end_;
    }

    virtual ElementType& Next() {
      return *m_iterator_++;
    }
  protected:
    T_STRING GetTieClassName() override {
      return _TSTR("CTieIteratorBase");
    }
  private:
    typename ContainerInstanceType::iterator m_iterator_;
    typename ContainerInstanceType::iterator m_iterator_end_;
  };

  template<typename E>
  class CTieVectorIterator : public CTieIteratorBase<std::vector<E>, E> {
  public:
    CTieVectorIterator(const typename std::vector<E>::iterator& begin, const typename std::vector<E>::iterator& end)
      : CTieIteratorBase<std::vector<E>, E>(begin, end) {}
  };

  template<typename K, typename V>
  class CTieUnorderedMapIterator : public CTieIteratorBase<std::unordered_map<K, V>, std::pair<const K, V>> {
  public:
    CTieUnorderedMapIterator(const typename std::unordered_map<K, V>::iterator& begin,
      const typename std::unordered_map<K, V>::iterator& end)
      : CTieIteratorBase<std::unordered_map<K, V>, std::pair<const K, V>>(begin, end) {}
  };

  std::string WStringToString(const std::wstring& ws);
  std::wstring StringToWString(const std::string& s);
  std::unique_ptr<const char[]> ConvertWStringToChars(const std::wstring& ws);
}
#endif