
#include "signal.h"

#include <cassert>

SigSlotBase::~SigSlotBase()
{
    while(!_bindings.empty()) {
        _bindings.front()->unbind();
    }
}

void SigSlotBase::add_binding(const std::shared_ptr<Binding>& b)
{
    _bindings.push_back(b);
}

void SigSlotBase::erase_binding(const std::shared_ptr<Binding>& b)
{
    auto pos = std::find(_bindings.begin(), _bindings.end(), b);
    if(pos == _bindings.end()) {
        throw std::runtime_error("Specified binding was not found");
    }
    _bindings.erase(pos);
}




Binding::Binding(SigSlotBase* emitter, SigSlotBase* receiver): _emitter(emitter), _receiver(receiver)
{
    assert(_emitter != nullptr);
    assert(_receiver != nullptr);
}

Binding::~Binding()
{
    unbind();
}

std::shared_ptr<Binding> Binding::create(SigSlotBase* em, SigSlotBase* recv)
{
    return std::shared_ptr<Binding>(new Binding(em, recv));
}

void Binding::unbind()
{
    // TODO: Don't unbind on the currently unbinding object (binding -> object -> binding loop)
    if(_emitter) {
        SigSlotBase* em = _emitter;
        _emitter = nullptr;
        em->erase_binding(shared_from_this());
    }
    if(_receiver) {
        SigSlotBase* recv = _receiver;
        _receiver = nullptr;
        recv->erase_binding(shared_from_this());
    }
}
