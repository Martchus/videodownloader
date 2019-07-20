#ifndef DOWNLOADRANGE_H
#define DOWNLOADRANGE_H

#include <QtGlobal>

namespace Network {

class DownloadRange {
public:
    DownloadRange();
    qint64 startOffset() const;
    bool setStartOffset(qint64 value = -1);
    qint64 currentOffset() const;
    bool setCurrentOffset(qint64 value = -1);
    void resetCurrentOffset();
    bool increaseCurrentOffset(qint64 bytes);
    qint64 endOffset() const;
    bool setEndOffset(qint64 value = -1);
    bool isUsedForRequest() const;
    void setUsedForRequest(bool value = true);
    bool isUsedForWritingOutput() const;
    void setUsedForWritingOutput(bool value = true);

private:
    qint64 m_startOffset;
    qint64 m_currentOffset;
    qint64 m_endOffset;
    bool m_useForRequest;
    bool m_useForWritingOutput;
};

/*!
 * \brief Returns the offset of the first byte to be downloaded.
 */
inline qint64 DownloadRange::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the current offset.
 */
inline qint64 DownloadRange::currentOffset() const
{
    return m_currentOffset;
}

/*!
 * \brief Resets the current offset (to the start offset).
 */
inline void DownloadRange::resetCurrentOffset()
{
    m_currentOffset = m_startOffset;
}

/*!
 * \brief Returns the offset of the last byte to be downloaded.
 */
inline qint64 DownloadRange::endOffset() const
{
    return m_endOffset;
}

/*!
 * \brief Returns an indication whether the range should be applied to the current request.
 */
inline bool DownloadRange::isUsedForRequest() const
{
    return m_useForRequest;
}

/*!
 * \brief Sets whether the range should be applied to the current request.
 */
inline void DownloadRange::setUsedForRequest(bool value)
{
    m_useForRequest = value;
}

/*!
 * \brief Returns an indication whether the range should be applied when writing the output file.
 */
inline bool DownloadRange::isUsedForWritingOutput() const
{
    return m_useForWritingOutput;
}

/*!
 * \brief Sets whether the range should be applied when writing the output file.
 */
inline void DownloadRange::setUsedForWritingOutput(bool value)
{
    m_useForWritingOutput = value;
}
} // namespace Network

#endif // DOWNLOADRANGE_H
