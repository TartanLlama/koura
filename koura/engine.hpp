#ifndef KOURA_ENGINE_HPP
#define KOURA_ENGINE_HPP

//C++
#include <iostream>
#include <optional>
#include <cctype>
#include <functional>
#include <cassert>

//Koura
#include "context.hpp"
#include "filters.hpp"

namespace koura {
    namespace {
        inline void stream_up_to_tag (std::istream& in, std::ostream& out) {
            char c;
            while (c = in.get(), in) {
                if (c == '{') {
                    auto next = in.peek();
                    if (next == '{' || next == '%') {
                        in.unget();
                        return;
                    }
                }
                out << c;
            }
        }

        inline void eat_whitespace (std::istream& in) {
            while (std::isspace(in.peek())) {
                in.get();
            }
        }

        inline char peek (std::istream& in) {
            eat_whitespace(in);
            return in.peek();
        }

        inline std::string get_identifier (std::istream& in) {
            eat_whitespace(in);
            std::string name = "";
            while (std::isalpha(in.peek()) || in.peek() == '_') {
                name += in.get();
            }

            return name;
        }

        inline std::optional<entity> parse_named_entity (std::istream& in, context& ctx) {
            auto name = get_identifier(in);
        
            auto ent = ctx.get_entity(std::string{name});

            eat_whitespace(in);

            if (ent.get_type() == entity::type::text) {
                return {ent};
            }
            else {
                auto next = in.peek();

                if (next == '.') {
                    if (ent.get_type() != entity::type::object) {
                        //TODO error
                    }

                    auto field_name = get_identifier(in);
                    auto ent_obj = ent.get_value<object_t>();

                    if (!ent_obj.count(field_name)) {
                        //TODO error
                    }

                    return {ent_obj.at(field_name)};
                }
                else if (next == '[') {
                    //TODO handle list access
                }
            }

            return {};
        }

        
        inline std::optional<koura::entity> parse_entity (std::istream& in, koura::context& ctx) {
            auto c = peek(in);
            //String literal
            if (c == '\'') {
                koura::text_t text;
                while (in.peek() != '\'') {
                    text += in.get();
                }
                return entity{text};
            }
            //Number literal
            else if (std::isdigit(c)) {
                koura::number_t num;
                in >> num;
                return entity{num};
            }
            //List literal
            else if (c == '[') {
                //TODO
            }
            //Named entity
            else {
                return parse_named_entity(in,ctx);
            }
        }
    
        inline void handle_if_expression (std::istream& in, std::ostream& out, context& ctx) {
            auto ent = parse_entity(in, ctx);

        
        }

        inline void handle_for_expression (std::istream& in, std::ostream& out, context& ctx) {

        }

        inline void handle_unless_expression (std::istream& in, std::ostream& out, context& ctx) {

        }

        inline void handle_set_expression (std::istream& in, std::ostream& out, context& ctx) {
            auto ent = parse_named_entity(in, ctx).value();
            auto val = parse_entity(in, ctx).value();

            switch (ent.get_type()) {
            case koura::entity::type::number:
                ent.get_value<koura::number_t>() = val.get_value<koura::number_t>();
                return;
            case koura::entity::type::text:
                ent.get_value<koura::text_t>() = val.get_value<koura::text_t>();
                return;
            case koura::entity::type::object:
                ent.get_value<koura::object_t>() = val.get_value<koura::object_t>();
                return;
            case koura::entity::type::list:
                //TODO
                return;
            }
        }
    }

    class engine {
    public:
        using expression_handler_t = std::function<void(std::istream&, std::ostream&, context&)>;
        using filter_t = std::function<std::string(std::string_view, context&)>;
        
        engine() :
            m_expression_handlers{
              {"if", handle_if_expression},
              {"unless", handle_unless_expression},
              {"set", handle_set_expression},
              {"for", handle_for_expression}
            },

            m_filters{
              {"capitalize", filters::capitalize}
            }
        {}
        
        void render (std::istream& in, std::ostream& out, context& ctx) {
            while (in) {
                stream_up_to_tag(in, out);
                if (in) {
                    process_tag(in, out, ctx);
                }
            }
        }
        
        void register_custom_expression (std::string_view name, expression_handler_t handler) {
            m_expression_handlers.emplace(std::string{name}, handler);
        }
        
        void register_custom_filter (std::string_view name, filter_t filter) {
            m_filters.emplace(std::string{name}, filter);
        }

    private:
        void process_tag (std::istream& in, std::ostream& out, context& ctx) {
            assert(in.get() == '{');

            auto next = in.get();

            if (next == '{') {
                eat_whitespace(in);
                handle_variable_tag(in,out,ctx);
                eat_whitespace(in);
                assert(in.get() == '}');
            }

            if (next == '%') {
                eat_whitespace(in);            
                handle_expression_tag(in,out,ctx);
                eat_whitespace(in);
                assert(in.get() == '%');
            }

            assert(in.get() == '}');
        }
        
        void handle_variable_tag (std::istream& in, std::ostream& out, context& ctx) {
            auto ent = parse_named_entity(in, ctx);
            if (!ent || ent.value().get_type() != entity::type::text) {
                //TODO error
            }

            auto text = ent.value().get_value<text_t>();
        
            eat_whitespace(in);
        
            while (in.peek() == '|') {
                in.get();
                text = handle_filter(in, ctx, text);
                eat_whitespace(in);
            }
        
            out << text;
        }
        
        void handle_expression_tag (std::istream& in, std::ostream& out, context& ctx) {
            std::string tag_name;
            in >> tag_name;

            if (!m_expression_handlers.count(tag_name)) {
                //TODO error
            }

            m_expression_handlers[tag_name](in,out,ctx);
        }
        
        auto handle_filter (std::istream& in, context& ctx, std::string_view text) -> std::string {
            auto filter_name = get_identifier(in);
        
            if (!m_filters.count(filter_name)) {
                //TODO error
            }

            return m_filters[filter_name](text,ctx);
        }
        
        std::unordered_map<std::string, expression_handler_t> m_expression_handlers;
        std::unordered_map<std::string, filter_t> m_filters;
    };

}

#endif
