#include "./downloadrange.h"

namespace Network {

/*!
 * \class DownloadRange
 * \brief The DownloadRange class defines the range of data to be downloaded.
 */

/*!
 * \brief Constructs a new download range.
 */
DownloadRange::DownloadRange()
    : m_startOffset(-1)
    , m_currentOffset(-1)
    , m_endOffset(-1)
    , m_useForRequest(true)
    , m_useForWritingOutput(true)
{
}

/*!
 * \brief Sets the offset of the first byte to be downloaded.
 * \returns Returns an indication whether the start offset could be updated.
 */
bool DownloadRange::setStartOffset(qint64 value)
{
    if (m_endOffset >= 0 && value >= 0 && m_endOffset < value) {
        return false;
    } else {
        m_startOffset = value;
        if (m_startOffset >= 0 && m_currentOffset < m_startOffset) {
            m_currentOffset = m_startOffset;
        }
    }
    return true;
}

/*!
 * \brief Sets the current offset. Must be between startOffset() and endOffset().
 * \returns Returns an indication whether the current offset could be updated.
 * \remarks Might be used to continue an interrupted download.
 */
bool DownloadRange::setCurrentOffset(qint64 value)
{
    if ((m_startOffset >= 0 && value < m_startOffset) || (m_endOffset >= 0 && value > m_endOffset)) {
        return false;
    } else {
        m_currentOffset = value;
    }
    return true;
}

/*!
 * \brief Increases the current offset by the specified number of \a bytes.
 * \returns Returns an indication whether the current offset could be increased.
 */
bool DownloadRange::increaseCurrentOffset(qint64 bytes)
{
    if (m_currentOffset >= 0) {
        return setCurrentOffset(m_currentOffset + bytes);
    } else if (m_startOffset >= 0) {
        return setCurrentOffset(m_startOffset + bytes);
    } else {
        return setCurrentOffset(bytes);
    }
}

/*!
 * \brief Sets the offset of the last byte to be downloaded.
 * \returns Returns an indication whether the end offset could be updated.
 * \remarks Might be used to continue an interrupted download.
 */
bool DownloadRange::setEndOffset(qint64 value)
{
    if (m_startOffset >= 0 && value >= 0 && value < m_startOffset)
        return false;
    else {
        m_endOffset = value;
        if (m_endOffset >= 0 && m_currentOffset > m_endOffset) {
            m_currentOffset = m_endOffset;
        }
    }
    return true;
}
} // namespace Network
