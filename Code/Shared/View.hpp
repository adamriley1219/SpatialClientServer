#pragma once
#include <improbable/worker.h>
#include <improbable/standard_library.h>



class View 
	: public worker::Dispatcher
{
public:
	template <typename... T>
	View(const worker::Components<T...>& components);
	~View() {};

	// Not copyable or movable.
	View(const View&) = delete;
	View(View&&) = delete;
	View& operator=(const View&) = delete;
	View& operator=(View&&) = delete;


public:
	struct entity_tracker_t
	{
		worker::Entity worker_entity;
		bool updated = false;
		bool created = true;
	};

	worker::Map<worker::EntityId, entity_tracker_t> m_entities;
	worker::Map<worker::EntityId, worker::Map<worker::ComponentId, worker::Authority>> m_component_authority;

private:	

	struct track_component_handler {
		View& view_reference;

		template <typename T>
		void Accept() const 
		{
			View& view = this->view_reference;

			view.OnAddComponent<T>([&view](const worker::AddComponentOp<T>& op) 
			{
				auto it = view.m_entities.find(op.EntityId);
				if (it != view.m_entities.end()) {
					entity_tracker_t& tracker = it->second;
					tracker.worker_entity.Add<T>(op.Data);
					tracker.updated = true;
				}
			});

			view.OnRemoveComponent<T>([&view](const worker::RemoveComponentOp& op) 
			{
				auto it = view.m_entities.find(op.EntityId);
				if (it != view.m_entities.end()) {
					entity_tracker_t& tracker = it->second;
					tracker.worker_entity.Remove<T>();
					tracker.updated = true;
				}
			});

			view.OnAddComponent<improbable::Metadata>([&view](const worker::AddComponentOp<improbable::Metadata> op)
			{
				auto it = view.m_entities.find(op.EntityId);
				if (it != view.m_entities.end()) {
					entity_tracker_t& tracker = it->second;
					tracker.worker_entity.Remove<T>();
					tracker.updated = true;
					tracker.created = true;
				}
			});

			view.OnAuthorityChange<T>([&view](const worker::AuthorityChangeOp& op) 
			{
				view.m_component_authority[op.EntityId][T::ComponentId] = op.Authority;
			});

			view.OnComponentUpdate<T>([&view](const worker::ComponentUpdateOp<T>& op) 
			{
				auto it = view.m_entities.find(op.EntityId);
				if (it != view.m_entities.end()) {
					entity_tracker_t& tracker = it->second;
					if (tracker.worker_entity.Get<T>()) {
						tracker.worker_entity.Update<T>(op.Update);
						tracker.updated = true;
					}
				}
			});
		}
	};


};

//--------------------------------------------------------------------------
/**
* View
*/
template <typename... T>
View::View(const worker::Components<T...>& components)
	: worker::Dispatcher{components}
{
	OnAddEntity([this](const worker::AddEntityOp& op) {
		m_entities[op.EntityId];
		m_component_authority[op.EntityId];
		});
	OnRemoveEntity([this](const worker::RemoveEntityOp& op) {
		m_entities.erase(op.EntityId);
		m_component_authority.erase(op.EntityId);
		});
	ForEachComponent(components, track_component_handler{ *this });
}
