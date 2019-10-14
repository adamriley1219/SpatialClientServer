#pragma once

struct Vertex_PCU;
class ActorRenderable;
class Camera;

void DrawDisc( const Vertex_PCU translation, float radius );
float GetDistanceBetween( const ActorRenderable* entityA, const ActorRenderable* entiryB );
float GetRandomlyChosenFloat( float a, float b );


