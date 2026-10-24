#include "android_environment_contract.h"

namespace tiecodec {

const std::vector<RequiredType>& AndroidEnvironmentContract::RequiredTypes() const {
  static const std::vector<RequiredType> types = {
      {"文本"},       {"整数"},     {"长整数"},     {"逻辑型"},     {"字符"},       {"字节"},
      {"单精度小数"}, {"小数"},     {"遍历器"},     {"键值表条目"}, {"异步调度器"}, {"安卓环境"},
      {"安卓窗口"},   {"窗口组件"}, {"自定义组件"}, {"组件容器"},   {"服务"},       {"广播接收器"},
      {"文本资源"},   {"主题资源"}, {"图片资源"},   {"应用"},
  };
  return types;
}

const std::vector<RequiredMember>& AndroidEnvironmentContract::RequiredMembers() const {
  static const std::vector<RequiredMember> members = {
      {"遍历器", "还有下一个"},
      {"遍历器", "下一个"},
      {"键值表条目", "键"},
      {"键值表条目", "值"},
      {"异步调度器", "创建单线程调度器"},
      {"异步调度器", "提交到全局调度器运行"},
      {"异步调度器", "结束提交到全局调度器运行"},
      {"异步调度器", "提交到调度器运行"},
      {"异步调度器", "结束提交到调度器运行"},
      {"异步调度器", "提交到主线程运行"},
      {"异步调度器", "结束提交到主线程运行"},
      {"窗口组件", "取安卓环境"},
      {"窗口组件", "创建完毕"},
      {"布局组件", "添加组件"},
  };
  return members;
}

}
