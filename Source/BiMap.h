#include <JuceHeader.h>

template <typename Key, typename Value>
class BiMap
{
public:
    // Insert or update both maps
    void set(const Key& k, const Value& v)
    {
        // If key already exists, remove its old value from reverse map
        if (forward.contains(k))
            reverse.remove(forward[k]);

        // If value already exists, remove its old key from forward map
        if (reverse.contains(v))
            forward.remove(reverse[v]);

        // Insert both ways
        forward.set(k, v);
        reverse.set(v, k);
    }



    // Lookup by value
Value* getValue(const Key& k)
{
    if (forward.contains(k))
        return &forward[k];  // safe because we know it exists
    return nullptr;
}

Key* getKey(const Value& v)
{
    if (reverse.contains(v))
    {
        return &reverse[v];  // safe because we know it exists
    }
    return nullptr;
}


    // Remove by value
    void removeByValue(const Value& v)
    {
        if (reverse.contains(v))
            forward.remove(reverse[v]);
        reverse.remove(v);
    }

private:
    juce::HashMap<Key, Value> forward;  // key -> value
    juce::HashMap<Value, Key> reverse;  // value -> key
};
