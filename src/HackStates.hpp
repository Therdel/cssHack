#pragma once
#include "Input.hpp"

namespace Hack {

    namespace Event {
        struct Activate {};
        struct Eject {};
        struct EnterMatch {};
        struct LeaveMatch {};
        struct UnbindKeys {};
        struct RebindKeys {};
    }

    // states
    namespace State {
        // just injected. Instantiate Input.
        class Injected;
        // Wait for activate / inject key.
        class Inactive;
        // instantiate Draw-Hook, GUI & hack-detours
        class Active;
        // keyhandlers active
        class InMatch;
        // keyhandlers removed
        class KeyBindingsDisabled;
    }

    class EventHandler {
    public:
        virtual ~EventHandler() noexcept = default;

        // events
        virtual auto handle(Event::Activate) -> void = 0;
        virtual auto handle(Event::Eject) -> void = 0;
        virtual auto handle(Event::EnterMatch) -> void = 0;
        virtual auto handle(Event::LeaveMatch) -> void = 0;
        virtual auto handle(Event::UnbindKeys) -> void = 0;
        virtual auto handle(Event::RebindKeys) -> void = 0;

        /*
        auto onInject() -> void;
        auto onEject() -> void;
        auto onEnterGame() -> void;
        auto onLeaveGame() -> void;
        auto onEnterMenu() -> void;
        auto onLeaveMenu() -> void;
        */
    };

    // problem: Wie sub-state wechseln, ohne superstate anzufassen?
    // Wir wollen States in ner Inheritance-Hierarchie, damit 
    // --> Wenn wir Polymorphische States wollen, brauchen wir doch 

    class State::Injected {
    private:
        Input _input;
    };

    // action: Wait for activate/inject combination before transition, wait 
    // alternative: Hold ScopedKeyHandler, which gets uninitialized upon state change.
    class State::Inactive {
    public:
        // needs Input as Parameter.
        // alternative: Should be able to access its superstate
        // by A: asking the FSM for current state
        //        > necessitates multiple active states -> state-stack
        //          > state-stack would need to operate polymorphically (no state information) or variadically(?) (closed set of states)
        //          > however, sub-state would know the super-state-type. Therefore there wouldn't be a hassle.
        //     B: using inheritance
        //        + polymorphic dispatch
        //        ? how to change substate without copying superstate object?
        //     C: being a composite variant of the parent
        //        + easy access to superstate fields
        //        ? how to implement polymorphic dispatch (no unnessecary restating of super-handlers)
        Inactive(Input &input)
            : _injectKeyBind{ input, key_inject, [&](SDL_KeyboardEvent const& event) { return onInjectKey(event); } }
        {}
    private:
        ScopedKeyHandler _injectKeyBind;

        // TODO: Implement state transition -> State::Active. Then return true.
        auto _onInjectKey(SDL_KeyboardEvent const& event) -> bool;
    };
    // instantiate Draw-Hook, GUI & hack-detours
    class Active;
    // keyhandlers active
    class InMatch;
    // keyhandlers removed
    class KeyBindingsDisabled;

    // machine
    class FSM {

    };
}
