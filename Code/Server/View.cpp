#include "Server/View.hpp"


//--------------------------------------------------------------------------
/**
* CleanupGarbage
*/
void View::CleanupGarbage()
{
	auto itr = m_entities.begin();
	while( itr != m_entities.end() )
	{
		if( itr->second.garbage )
		{
			itr = m_entities.erase( itr );
		}
		else
		{
			++itr;
		}
	}
}
