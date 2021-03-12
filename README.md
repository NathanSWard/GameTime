#WIP Game Engine 

Example Usage:

```cpp
void my_startup_system(Commands cmds, Resource<AssetServer> server)
{
	auto texture_handle = server.load<Texture>("my-image.png"); // async loading of the image
	cmds.spawn(SpriteBundle {
			.sprite = Sprite {
				.size = Vec2(10.f, 10.f),
			},
			.texture = texture_handle,
		});
}

void my_system(
	Query<With<Transform, Visible const>> query,
	Resource<Input<KeyCode>> inputs)
{
	query.each([&](Transform& tform) {
		if (inputs->pressed(KeyCode::A)) {
			tform.translation.x() -= 5; 
		}
		else if (inputs->pressed(KeyCode::D)) {
			tform.translation.x() += 5;
		}
	});
}

int main()
{
	GameBuilder
		.add_plugin(DefaultPlugins{}) // adds pluings such as: Sprite, Render, Asset, Input, (and more...)
		.add_system(my_system)
		.add_startup_system(my_startup_system)
		.build()
		.run();
}
```