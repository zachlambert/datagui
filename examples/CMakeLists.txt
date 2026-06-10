function(create_example directory name)
  set(full_name example_${directory}_${name})
  add_executable(${full_name} ${directory}/${name}.cpp)
  target_link_libraries(${full_name} datagui)
endfunction()


create_example(gui basic)
create_example(gui core_elements)
create_example(gui extra_inputs)
create_example(gui lists)
# create_example(gui popups)
# create_example(gui splits)

# create_example(datapack datapack)
# create_example(datapack datapack_2)

# create_example(canvas canvas2d)
# create_example(canvas canvas3d)

# create_example(plotter plotter)
# create_example(plotter standalone)
