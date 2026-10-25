#include "ast.h"

#include "constants.h"
#include "text_util.h"

namespace tiecodec {

const AnnotationArgument* Annotation::FindArgument(const std::string& argument_name) const {
  for (const AnnotationArgument& argument : arguments) {
    if (argument.name == argument_name)
      return &argument;
  }
  return nullptr;
}

std::string Annotation::StringArgument(const std::string& argument_name) const {
  const AnnotationArgument* argument = FindArgument(argument_name);
  return argument == nullptr ? std::string() : argument->value;
}

std::string Annotation::TextArgument(const std::string& argument_name) const {
  const AnnotationArgument* argument = FindArgument(argument_name);
  if (argument == nullptr && argument_name == kArgumentParameter && !arguments.empty())
    argument = &arguments.front();
  return argument == nullptr ? std::string() : Unescape(UnwrapQuotes(argument->value));
}

std::string Annotation::UnwrappedArgument(const AnnotationArgument& argument) const {
  return Unescape(UnwrapQuotes(argument.value));
}

const Annotation* Unit::FindAnnotation(const std::string& annotation_name) const {
  for (const Annotation& annotation : annotations) {
    if (annotation.name == annotation_name)
      return &annotation;
  }
  return nullptr;
}

const Annotation* ClassDecl::FindAnnotation(const std::string& annotation_name) const {
  for (const Annotation& annotation : annotations) {
    if (annotation.name == annotation_name)
      return &annotation;
  }
  return nullptr;
}

bool ClassDecl::HasAnnotation(const std::string& annotation_name) const {
  return FindAnnotation(annotation_name) != nullptr;
}

}
