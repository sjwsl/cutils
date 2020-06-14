#pragma once

namespace cutils {

template<typename Tag>
struct result {
    typedef typename Tag::type type;
    static type ptr;
};

template<typename Tag>
typename result<Tag>::type result<Tag>::ptr;

template<typename Tag, typename Tag::type p>
struct rob : result<Tag> {
    struct filler {
        filler() { result<Tag>::ptr = p; }
    };
    static filler filler_obj;
};

template<typename Tag, typename Tag::type p>
typename rob<Tag, p>::filler rob<Tag, p>::filler_obj;

} // namespace cutils
