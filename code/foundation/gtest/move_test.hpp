#pragma once
class MoveTest
{
public:
    MoveTest()                = default;
    ~MoveTest()               = default;
    MoveTest(const MoveTest&) = default;
    MoveTest(MoveTest&& obj) noexcept : _moveConstruct(true) { obj._moveConstruct = true; }
    MoveTest& operator=(const MoveTest&) = default;
    MoveTest& operator=(MoveTest&& obj) noexcept
    {
        _moveAssign     = true;
        obj._moveAssign = true;
        return *this;
    }
    bool MoveConstruct() const { return _moveConstruct; }
    bool MoveAssign() const { return _moveAssign; }
    bool Moved() const { return _moveConstruct || _moveAssign; }
    void Reset()
    {
        _moveConstruct = false;
        _moveAssign    = false;
    }
private:
    bool _moveConstruct = false;
    bool _moveAssign    = false;
};
