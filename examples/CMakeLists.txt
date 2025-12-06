function(create_example name)
    add_executable(example_${name} ${name}.cpp)
    target_link_libraries(example_${name} datagui)
endfunction()

create_example(window)

create_example(gui_1)
create_example(gui_2)
create_example(gui_3)
create_example(gui_4)
create_example(gui_5)
create_example(gui_6)

# create_example(datapack)

create_example(canvas2)
