#pragma once
namespace RHEngine {
	/**
	 * @brief Abstract class for state caching to reduce state set calls.
	 * 
	 * We cache state changes and don't send them to GPU as long as needed, 
	 * because any set call to the same part of pipeline twice before draw call
	 * will override previous state, and spend lots of time doing some API stuff twice,
	 * which is indeed big CPU overhead and should be avoided
	 */
	class IStateCacheObject
	{
	public:
		/**
		 * @brief Checks if state has been changed and should be updated
		 * 
		 * @return true if state has been changed
		 * @return false if state has not changed
		 */
		virtual bool IsDirty();

		/**
		 * @brief Signals that state has been changed and should be updated
		 * 
		 */
		virtual void MakeDirty();

		/**
		 * @brief Updates state if required
		 * 
		 * @param deviceObject - API-specific device object that will update state
		 */
		virtual void Flush(void* deviceObject);

		/**
		 * @brief Callback on state update
		 * 
		 * @param deviceObject - API-specific device object that will update state
		 */
		virtual void OnFlush(void* deviceObject)=0;
	private:
		bool m_bDirty = true;
	};
};