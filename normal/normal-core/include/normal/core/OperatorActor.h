//
// Created by Matt Youill on 31/12/19.
//

#ifndef NORMAL_OPERATORACTOR_H
#define NORMAL_OPERATORACTOR_H

#include <memory>

#include <caf/all.hpp>

#include "normal/core/Message.h"
#include "normal/core/Operator.h"
#include "normal/core/StartMessage.h"

namespace normal::core {

class Operator;

class OperatorActor : public caf::event_based_actor {

private:
  std::shared_ptr<normal::core::Operator> opBehaviour_;

public:
  OperatorActor(caf::actor_config &cfg, std::shared_ptr<normal::core::Operator> opBehaviour);

  std::shared_ptr<normal::core::Operator> operator_() const;

  caf::behavior make_behavior() override;

};

}

#endif //NORMAL_OPERATORACTOR_H