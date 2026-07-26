// SPDX-FileCopyrightText: 2023 flowln <flowlnlnln@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "ShaderPackModel.h"

#include <QMessageBox>
#include <utility>

namespace ResourceDownload {

ShaderPackResourceModel::ShaderPackResourceModel(const BaseInstance& base_inst,
                                                 ResourceAPI* api,
                                                 const QString& debugName,
                                                 QString metaEntryBase)
    : ResourceModel(api), m_base_instance(base_inst), m_debugName(debugName + " (Model)"), m_metaEntryBase(std::move(metaEntryBase))
{}

/******** Make data requests ********/

ResourceAPI::SearchArgs ShaderPackResourceModel::createSearchArguments()
{
    auto sort = getCurrentSortingMethodByIndex();
    ResourceAPI::SearchArgs args{};
    args.type = ModPlatform::ResourceType::ShaderPack;
    args.offset = m_next_search_offset;
    args.search = m_search_term;
    args.sorting = sort;
    return args;
}

ResourceAPI::VersionSearchArgs ShaderPackResourceModel::createVersionsArguments(const QModelIndex& entry)
{
    auto pack = m_packs[entry.row()];
    ResourceAPI::VersionSearchArgs args{};
    args.pack = pack;
    args.resourceType = ModPlatform::ResourceType::ShaderPack;
    return args;
}

ResourceAPI::ProjectInfoArgs ShaderPackResourceModel::createInfoArguments(const QModelIndex& entry)
{
    auto pack = m_packs[entry.row()];
    ResourceAPI::ProjectInfoArgs args{};
    args.pack = pack;
    return args;
}

void ShaderPackResourceModel::searchWithTerm(const QString& term, unsigned int sort)
{
    if (m_search_term == term && m_search_term.isNull() == term.isNull() && m_current_sort_index == sort) {
        return;
    }

    setSearchTerm(term);
    m_current_sort_index = sort;

    refresh();
}

}  // namespace ResourceDownload
